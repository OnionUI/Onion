import React from 'react';
import clsx from 'clsx';
import LinkItem from '@docusaurus/theme-classic/lib/theme/Footer/LinkItem'
import styles from './styles.module.scss';

type FeatureItem = {
  title: string | JSX.Element;
  icon: string;
  description: JSX.Element;
};

const FeatureList: FeatureItem[] = [
  {
    title: 'GameSwitcher',
    icon: require('@site/static/img/icons/gameswitcher.png').default,
    description: (
      <>
        Quickly switch between games by the press of a button. Resume where you left off in an instant.
      </>
    ),
  },
  {
    title: <LinkItem item={{ label: "Ports Collection", href: "https://github.com/OnionUI/Ports-Collection/blob/main/README.md" }} />,
    icon: require('@site/static/img/icons/ports.png').default,
    description: (
      <>
        Our Ports Collection offers a native gaming experience on the Miyoo Mini.
      </>
    ),
  },
  {
    title: <LinkItem item={{ label: "Themes", href: "https://github.com/OnionUI/Themes/blob/main/README.md" }} />,
    icon: require('@site/static/img/icons/themes.png').default,
    description: (
      <>
        An extensive collection of community-made themes, enabling you to completely change the feel of the device.
      </>
    ),
  },
  {
    title: 'RetroArch',
    icon: require('@site/static/img/icons/retroarch.png').default,
    description: (
      <>
        We're using our own build of RetroArch, with a custom display driver made for maximum precision and performance.
      </>
    ),
  },
  {
    title: 'Activity Tracker',
    icon: require('@site/static/img/icons/activity.png').default,
    description: (
      <>
        Our built-in activity tracker allows you to keep an eye on your play activity. See your most played games rated on total play time, average play time, and number of play sessions.
      </>
    ),
  },
  {
    title: 'Search',
    icon: require('@site/static/img/icons/search.png').default,
    description: (
      <>
        Once you've added your vast collection of roms, use our global search function to easily find the game you're looking for.
      </>
    ),
  },
];

function Feature({ title, icon, description }: FeatureItem) {
  return (
    <div className={clsx('col col--4')}>
      <div className="text--center">
        <img src={icon} />
      </div>
      <div className="text--center padding-horiz--md">
        <h3>{title}</h3>
        <p>{description}</p>
      </div>
    </div>
  );
}

export default function HomepageFeatures(): JSX.Element {
  return (
    <section className={styles.features}>
      <div className="container">
        <div className="row">
          {FeatureList.map((props, idx) => (
            <Feature key={idx} {...props} />
          ))}
        </div>
      </div>
    </section>
  );
}
