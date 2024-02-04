import React from 'react';
import clsx from 'clsx';
import Link from '@docusaurus/Link';
import useDocusaurusContext from '@docusaurus/useDocusaurusContext';
import Layout from '@theme/Layout';
import HomepageFeatures from '@site/src/components/HomepageFeatures';
import ReleaseLink from '@site/src/components/ReleaseLink';

import styles from './styles.module.scss';
import { Content } from "@theme/BlogPostPage";

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
                    <ReleaseLink showDownloads className="button button--primary button--lg" url="https://api.github.com/repos/OnionUI/Onion/releases/latest" />
                    <ReleaseLink showDownloads className="button button--secondary button--lg" url="https://api.github.com/repos/OnionUI/Onion/releases/tags/latest" />
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

function RecentPosts({ recentPosts }: Props): JSX.Element {
    return (<>
        {recentPosts && (
            <section className={clsx('container padding--lg', styles.recentPosts)}>
                <h2>Recent blog posts</h2>
                <Link href="/blog">See all blog posts</Link>
                <div className="row">
                    {recentPosts.map(({ content }) => (
                        <article key={content.metadata.permalink} className="col col--4">
                            <a className="card margin-vert--lg" href={content.metadata.permalink}>
                                {content.assets.image && (
                                    <div className="card__image">
                                        <img src={content.assets.image} />
                                    </div>
                                )}
                                <div className="card__header">
                                    <h3>{content.metadata.title}</h3>
                                </div>
                                <div className="card__body">{content.metadata.description}</div>
                                <div className={clsx('card__footer', styles.card__footer)}>
                                    <small>{content.metadata.formattedDate}</small>
                                </div>
                            </a>
                        </article>
                    ))}
                </div>
            </section>
        )}
    </>);
}

interface Props {
    readonly recentPosts: readonly { readonly content: Content }[];
}

export default function Home({ recentPosts }: Props): JSX.Element {
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
                <RecentPosts recentPosts={recentPosts} />
            </main>
        </Layout>
    );
}
