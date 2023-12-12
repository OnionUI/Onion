import React from 'react';
import Link from '@docusaurus/Link';
import Icon from "@material-ui/core/Icon";

type IconLinkProps = {
    children: any;
    href: string;
    icon?: string;
};

export default function IconLink({ children, href, icon }: IconLinkProps): JSX.Element {
    return (
        <Link href={href}><Icon>{icon}</Icon><span>{children}</span></Link>
    );
}
