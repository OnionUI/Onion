import React from 'react';
import Link from '@docusaurus/Link';

interface Props {
    theme: string
    colWidth: number
}

export function ThemeCard({ theme, colWidth = 3 }: Props): JSX.Element {
    const [name, authorString] = theme.split(" by ");
    const authors = authorString.split(" + ");
    const preview = `https://raw.githubusercontent.com/OnionUI/Themes/main/themes/${encodeURIComponent(theme)}/preview.png`;
    const download = `https://github.com/OnionUI/Themes/blob/main/release/${encodeURIComponent(theme)}.zip?raw=true`;

    return (
        <article className={`col col--${colWidth} padding-horiz--xs`}>
            <div className="card margin-bottom--md" style={{ maxWidth: 480, margin: "0 auto" }}>
                <div className="card__image text--center">
                    <img src={preview} />
                </div>
                <div className="card__body text--center padding-vert--none text--truncate">
                    <small><Link href={download} title={theme}><b>{name}</b></Link></small>
                </div>
                <div className="card__footer text--center padding-top--none" style={{ lineHeight: 1 }}>
                    <small><small><i>{authors.map((author, index) => (
                        <span key={index}>{author}<br /></span>
                    ))}</i></small></small>
                </div>
            </div>
        </article >
    )
}

export default ThemeCard;
