import React from 'react';
import { useColorMode } from '@docusaurus/theme-common';
import Link from '@docusaurus/Link';

import styles from './styles.module.scss';

import GithubMark from './assets/github-mark.svg';
import GithubMarkWhite from './assets/github-mark-white.svg';
import RedditMark from './assets/reddit-logo-2436.svg';
import DiscordMarkWhite from './assets/discord-mark-white.svg';
import DiscordMarkBlue from './assets/discord-mark-blue.svg';

const marks = {
    light: {
        github: GithubMark,
        discord: DiscordMarkBlue,
        reddit: RedditMark
    },
    dark: {
        github: GithubMarkWhite,
        discord: DiscordMarkWhite
    }
}

function getMark(type: string, colorMode: string): JSX.Element {
    const Mark = marks[colorMode][type] || marks.light[type];
    return (<Mark />);
}

export interface SocialLinkInfo {
    user: string
    id?: string
}

interface SocialLinkProps extends SocialLinkInfo {
    type: string
}

export function SocialLink({ type, user, id }: SocialLinkProps): JSX.Element {
    const { colorMode } = useColorMode();

    const href = {
        github: () => `https://github.com/${user}`,
        discord: () => `https://discordapp.com/users/${id}`,
        reddit: () => `https://reddit.com/user/${user}`
    }[type] || (() => null);

    const mark = getMark(type, colorMode)

    return (<Link href={href()} className={styles.socialLink} title={user}>{mark}</Link>)
}

export default SocialLink;
