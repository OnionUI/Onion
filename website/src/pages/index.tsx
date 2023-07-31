import React, { useEffect, useState } from 'react';
import clsx from 'clsx';
import Link from '@docusaurus/Link';
import useDocusaurusContext from '@docusaurus/useDocusaurusContext';
import Layout from '@theme/Layout';
import HomepageFeatures from '@site/src/components/HomepageFeatures';

import styles from './index.module.scss';

type ReleaseProps = {
  url: string;
  label: string;
  isPrimary?: boolean
};

function ReleaseLink({ url, label, isPrimary }: ReleaseProps) {
  const [data, setData] = useState([]);

  useEffect(() => {
    fetch(url)
      .then(response => response.ok ? response.json() : null)
      .then(setData);
  }, []);

  return (
    <>
      {data &&
        <Link
          className={`button button--${isPrimary ? 'primary' : 'secondary'} button--lg`}
          href={data['html_url']}>
          {data['name']} ({label})
        </Link>
      }
    </>
  );
}

function HomepageHeader() {
  const { siteConfig } = useDocusaurusContext();

  return (
    <header className={clsx('hero hero--primary', styles.heroBanner)}>
      <div className="container">
        <h1 className="hero__title">A Fresh Onion for You!</h1>
        <p className="hero__subtitle">{siteConfig.tagline}</p>
        <div className={styles.buttons}>
          <ReleaseLink url="https://api.github.com/repos/OnionUI/Onion/releases/latest" label='Stable' isPrimary />
          <ReleaseLink url="https://api.github.com/repos/OnionUI/Onion/releases/tags/latest" label='Beta' />
        </div>
      </div>
    </header>
  );
}

export default function Home(): JSX.Element {
  const { siteConfig } = useDocusaurusContext();
  return (
    <Layout
      title={`Welcome`}
      description={`${siteConfig.tagline}`}>
      <HomepageHeader />
      <main>
        <div className='container'>
          <div className='row'>
            <div className='col text--center padding--lg'>
              <b>Windows user?</b> Try <Link href='https://github.com/schmurtzm/Onion-Desktop-Tools/blob/main/README.md'>Onion Desktop Tools</Link> for easy SD card preparation and installation.
            </div>
          </div>
        </div>
        <HomepageFeatures />
      </main >
    </Layout >
  );
}
