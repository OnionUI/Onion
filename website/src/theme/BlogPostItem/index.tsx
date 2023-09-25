import React from 'react';
import { useColorMode } from '@docusaurus/theme-common';
import { useBlogPost } from '@docusaurus/theme-common/internal';
import BlogPostItem from '@theme-original/BlogPostItem';
import type BlogPostItemType from '@theme/BlogPostItem';
import type { WrapperProps } from '@docusaurus/types';

import Giscus from '@giscus/react';

type Props = WrapperProps<typeof BlogPostItemType>;

export default function BlogPostItemWrapper(props: Props): JSX.Element {
  const { colorMode } = useColorMode();
  const { isBlogPostPage, metadata } = useBlogPost();
  const { frontMatter } = metadata;
  const { comments = true } = frontMatter;

  return (
    <>
      <BlogPostItem {...props} />
      {isBlogPostPage && comments && (
        <Giscus
          id="comments"
          repo="OnionUI/onionui.github.io"
          repoId="R_kgDOI2kfVQ"
          category="General"
          categoryId="DIC_kwDOI2kfVc4CZNyC"
          mapping="pathname"
          reactionsEnabled="1"
          emitMetadata="0"
          inputPosition="bottom"
          theme={colorMode}
          lang="en"
          loading="lazy"
        />
      )}
    </>
  );
}
