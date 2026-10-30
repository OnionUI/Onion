"""Host-side OTA transport regression tests; no network or device access."""
import hashlib
import json
import os
from pathlib import Path
import re
import ssl
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / 'static/build/.tmp_update/script/ota_update.sh'
SOURCE = SCRIPT.read_text()


def function(name):
    match = re.search(r'^' + name + r'\(\) \{\n.*?^\}', SOURCE, re.M | re.S)
    if not match:
        raise AssertionError('Missing OTA function: ' + name)
    return match.group(0)


class TransportTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix='onion-ota-')
        self.addCleanup(self.temp.cleanup)
        self.path = Path(self.temp.name)
        self.sysdir = self.path / 'system'
        (self.sysdir / 'script').mkdir(parents=True)
        choice = self.sysdir / 'script/shellect.sh'
        choice.write_text('#!/bin/sh\nprintf "Yes\\n"\n')
        choice.chmod(0o755)
        mock = self.path / 'curl'
        mock.write_text('''#!/usr/bin/env python3
import json, os, pathlib, sys
args = sys.argv[1:]
pathlib.Path(os.environ['CURL_LOG']).write_text(json.dumps(args))
status = int(os.environ.get('CURL_EXIT', '0'))
if status:
    print('curl: (%d) simulated transport error' % status, file=sys.stderr)
    sys.exit(status)
if '--output' in args:
    output = args[args.index('--output') + 1]
    if output != '/dev/null':
        pathlib.Path(output).write_bytes(b'zip-data')
''')
        mock.chmod(0o755)
        self.env = dict(os.environ, PATH=str(self.path) + os.pathsep + os.environ['PATH'],
                        CURL_LOG=str(self.path / 'curl.json'), OTA_TEST_DIR=str(self.sysdir))

    def run_function(self, name, status=0, size=8):
        env = dict(self.env, CURL_EXIT=str(status))
        harness = '''
sysdir="$OTA_TEST_DIR"
read() { :; }
clear() { :; }
sleep() { :; }
sync() { :; }
fsck.fat() { :; }
Release_Version=4.3.1-1
Release_url=https://github.com/OnionUI/Onion/releases/download/example/Onion.zip
'''
        harness += '\nRelease_size=%d\n' % size
        harness += function(name) + '\n' + name + '\necho TRANSPORT_COMPLETED\n'
        return subprocess.run(['bash', '-c', harness], env=env, cwd=self.path,
                              capture_output=True, text=True)

    def args(self):
        return json.loads((self.path / 'curl.json').read_text())

    def test_connection_uses_explicit_ca_and_verifies_https(self):
        result = self.run_function('check_connection')
        self.assertEqual(result.returncode, 0, result.stderr)
        args = self.args()
        self.assertEqual(args[args.index('--cacert') + 1], str(self.sysdir / 'certificates/cacert.pem'))
        self.assertNotIn('--insecure', args)
        self.assertNotIn('-k', args)
        self.assertIn('--max-time', args)
        self.assertIn('https://github.com', args)

    def test_certificate_error_is_visible_and_stops_connection_check(self):
        for code in (60, 77):
            with self.subTest(code=code):
                result = self.run_function('check_connection', status=code)
                self.assertEqual(result.returncode, 2)
                self.assertIn('curl: (%d)' % code, result.stderr)
                self.assertNotIn('TRANSPORT_COMPLETED', result.stdout)

    def test_download_follows_redirects_and_keeps_size_check(self):
        result = self.run_function('download_update')
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn('--location', self.args())
        self.assertIn('--fail', self.args())
        self.assertIn('--cacert', self.args())
        self.assertEqual((self.sysdir / 'download/4.3.1-1.zip').read_bytes(), b'zip-data')
        self.assertIn('File size OK!', result.stdout)

    def test_failed_download_cannot_continue_to_apply(self):
        result = self.run_function('download_update', status=22)
        self.assertEqual(result.returncode, 5)
        self.assertIn('No update has been applied', result.stdout)
        self.assertNotIn('Download done', result.stdout)
        self.assertNotIn('TRANSPORT_COMPLETED', result.stdout)

    def test_wrong_download_size_still_stops_update(self):
        result = self.run_function('download_update', size=100)
        self.assertEqual(result.returncode, 5)
        self.assertIn('Wrong download size', result.stdout)
        self.assertNotIn('TRANSPORT_COMPLETED', result.stdout)

    def test_bundled_ca_file_parses_and_matches_documented_checksum(self):
        certs = ROOT / 'static/build/.tmp_update/certificates'
        data = (certs / 'cacert.pem').read_bytes()
        self.assertIn(hashlib.sha256(data).hexdigest(), (certs / 'README.md').read_text())
        context = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
        context.load_verify_locations(cafile=str(certs / 'cacert.pem'))
        self.assertGreater(context.cert_store_stats()['x509_ca'], 100)


if __name__ == '__main__':
    unittest.main()
