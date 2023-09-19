import React from 'react';
import BrowserOnly from '@docusaurus/BrowserOnly';
import Copyright from '@theme-original/Footer/Copyright';
import type CopyrightType from '@theme/Footer/Copyright';
import type { WrapperProps } from '@docusaurus/types';
import PageCount from '@site/src/components/PageCount';

type Props = WrapperProps<typeof CopyrightType>;

export default function CopyrightWrapper(props: Props): JSX.Element {
  return (
    <>
      <Copyright {...props} />
      <BrowserOnly>
        {() => (
          <PageCount path="OnionUI/onionui.github.io" />
        )}
      </BrowserOnly>
    </>
  );
}
