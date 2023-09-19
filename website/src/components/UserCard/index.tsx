import React from 'react';
import styles from './styles.module.scss';

import { SocialLink, SocialLinkInfo } from '../SocialLink';

interface Props {
    user: string
    title: string
    aka: string
    reddit?: string
    discord?: SocialLinkInfo
    children: any
}

export default function UserCard({ user, title, aka, reddit, discord, children }: Props): JSX.Element {
    return (
        <article className={styles.userCard}>
            <div className="card">
                <div className="card__image">
                    <img src={`https://github.com/${user}.png`} />
                </div>
                <div className="card__header">
                    <h3>{aka || user}</h3><i>{title}</i>
                </div>
                <div className="card__body">{children}</div>
                <div className="card__footer">
                    <SocialLink type="github" user={user} />
                    {discord && <SocialLink type="discord" {...discord} />}
                    {reddit && <SocialLink type="reddit" user={reddit} />}
                </div>
            </div>
        </article>
    )
}
