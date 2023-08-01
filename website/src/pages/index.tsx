import React from 'react';
import clsx from 'clsx';
import Link from '@docusaurus/Link';
import useDocusaurusContext from '@docusaurus/useDocusaurusContext';
import Layout from '@theme/Layout';
import HomepageFeatures from '@site/src/components/HomepageFeatures';
import ReleaseLink from '@site/src/components/ReleaseLink';

import styles from './index.module.scss';

function HomepageHeader() {
    const { siteConfig } = useDocusaurusContext();

    return (
        <header className={clsx('hero hero--primary', styles.heroBanner)}>
            <div className="container">
                <h1 className="hero__title">
                    <span className="keep-together">A Fresh Onion</span> <span className="keep-together">for You!</span>
                </h1>
                <p className="hero__subtitle">{siteConfig.tagline}</p>
                <div className={styles.buttons}>
                    <ReleaseLink label="Stable" showDownloads className="button button--primary button--lg" url="https://api.github.com/repos/OnionUI/Onion/releases/latest" />
                    <ReleaseLink label="Beta" showDownloads className="button button--secondary button--lg" url="https://api.github.com/repos/OnionUI/Onion/releases/tags/latest" />
                </div>
            </div>
        </header>
    );
}

function CenteredNote({ children }) {
    return (
        <div className='container'>
            <div className='row'>
                <div className='col text--center padding--lg'>
                    {children}
                </div>
            </div>
        </div>
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
                <CenteredNote>
                    <b>Windows user?</b> Try <Link href='https://github.com/schmurtzm/Onion-Desktop-Tools/blob/main/README.md'>Onion Desktop Tools</Link> for easy SD card preparation and installation.
                </CenteredNote>
                <HomepageFeatures />
            </main >
        </Layout >
    );
}
