import React from 'react';
export * from './ThemeCard';

interface Props {
    columns: number
    children: JSX.Element[]
}

export default function ThemeCollection({ columns = 4, children }: Props): JSX.Element {
    const colWidth = Math.round(12 / columns);
    const renderChildren = () => React.Children.map(children, child =>
        React.cloneElement(child, { colWidth }));

    return (<div className="row">{renderChildren()}</div>)
}
