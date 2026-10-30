# OTA CA certificates

`cacert.pem` is Mozilla's public root certificate bundle converted by curl's
mk-ca-bundle tool, downloaded unchanged over verified HTTPS from
https://curl.se/ca/cacert.pem. It is used explicitly by the OTA script, including
on devices whose internal firmware lacks a usable CA store.

- Mozilla snapshot: 2026-10-20; 118 certificates.
- File SHA-256: `796f1389bfea13c0ed78f663b765baaff54a508e5483786bd6ece73c5e3e3cbe`.
- Conversion and update information: https://curl.se/docs/caextract.html.
- Source: https://raw.githubusercontent.com/mozilla-firefox/firefox/refs/heads/release/security/nss/lib/ckfw/builtins/certdata.txt.
- License: MPL 2.0, included in `LICENSE-MPL-2.0.txt`.

To refresh, download the bundle and https://curl.se/ca/cacert.pem.sha256 over
verified HTTPS, compare the published SHA-256 with the downloaded file, and update
the snapshot date, count and checksum above. Ship the bundle with the script;
never fetch trust roots over an unverified connection on the device.

Regression checks from the repository root:

```sh
sh -n static/build/.tmp_update/script/ota_update.sh
PYTHONDONTWRITEBYTECODE=1 python3 -m unittest discover -s test -p test_ota_transport.py -v
```

The host tests mock network and device operations. Hardware OTA success must be
checked separately on a Mini Plus; host tests cannot establish that result.
