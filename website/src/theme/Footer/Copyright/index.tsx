import React from 'react';
import Copyright from '@theme-original/Footer/Copyright';
import type CopyrightType from '@theme/Footer/Copyright';
import type { WrapperProps } from '@docusaurus/types';
import PageCount from '@site/src/components/PageCount';

type Props = WrapperProps<typeof CopyrightType>;

export default function CopyrightWrapper(props: Props): JSX.Element {
  return (
    <>
      <Copyright {...props} />
      <PageCount user="OnionUI" repo="onionui.github.io" />
    </>
  );
}
