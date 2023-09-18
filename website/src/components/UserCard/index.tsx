import React from 'react';
import styles from './styles.module.scss';

interface Props {
    user: string
    title: string
    aka: string
    children: any
}

export default function UserCard({ user, title, aka, children }: Props): JSX.Element {
    return (
        <article className={styles.userCard}>
            <a className="card" href={`https://github.com/${user}`}>
                <div className="card__image">
                    <img src={`https://github.com/${user}.png`} />
                </div>
                <div className="card__header">
                    <h3>{aka || user}</h3><i>{title}</i>
                </div>
                <div className="card__body">{children}</div>
            </a>
        </article>
    )
}
