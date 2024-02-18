// @ts-check
// Note: type annotations allow type checking and IDEs autocompletion

const lightCodeTheme = require('prism-react-renderer/themes/github');
const darkCodeTheme = require('prism-react-renderer/themes/dracula');

const remarkPing = require('remark-ping');
const remarkPingOpts = {
  pingUsername: (username) => true,
  userURL: (username) => `https://github.com/${username}`
}

const oembed = require('remark-oembed');
const oembedOpts = {
  syncWidget: true
}

/** @type {import('@docusaurus/types').Config} */
const config = {
  title: 'Onion',
  tagline: 'OS overhaul for Miyoo Mini and Mini+',
  favicon: 'img/favicon.png',
  trailingSlash: false,

  // Set the production url of your site here
  url: 'https://onionui.github.io',
  // Set the /<baseUrl>/ pathname under which your site is served
  // For GitHub pages deployment, it is often '/<projectName>/'
  baseUrl: '/',

  // GitHub pages deployment config.
  // If you aren't using GitHub pages, you don't need these.
  organizationName: 'OnionUI', // Usually your GitHub org/user name.
  projectName: 'onionui.github.io', // Usually your repo name.
  deploymentBranch: 'main',

  onBrokenLinks: 'throw',
  onBrokenMarkdownLinks: 'warn',

  // Even if you don't use internalization, you can use this field to set useful
  // metadata like html lang. For example, if your site is Chinese, you may want
  // to replace "en" with "zh-Hans".
  i18n: {
    defaultLocale: 'en',
    locales: ['en'],
  },

  presets: [
    [
      'classic',
      /** @type {import('@docusaurus/preset-classic').Options} */
      ({
        docs: {
          sidebarPath: require.resolve('./sidebars.js'),
          editUrl: 'https://github.com/OnionUI/Onion/edit/main/website',
          lastVersion: 'current',
          versions: {
            current: {
              label: '4.3'
            },
          },
          remarkPlugins: [
            [oembed, oembedOpts],
            [remarkPing, remarkPingOpts],
          ],
        },
        blog: false,
        theme: {
          customCss: require.resolve('./src/style/custom.scss'),
        },
      }),
    ],
  ],

  stylesheets: [
    "https://fonts.googleapis.com/icon?family=Material+Icons",
  ],

  themes: [
    [
      // @ts-ignore
      require.resolve("@easyops-cn/docusaurus-search-local"),
      /** @type {import("@easyops-cn/docusaurus-search-local").PluginOptions} */
      // @ts-ignore
      ({
        hashed: true,
        language: ["en"],
        highlightSearchTermsOnTargetPage: false,
        explicitSearchResultPath: true,
        removeDefaultStopWordFilter: true
      }),
    ],
  ],

  themeConfig:
    /** @type {import('@docusaurus/preset-classic').ThemeConfig} */
    ({
      image: 'img/onion_banner.png',
      navbar: {
        logo: {
          alt: 'Onion Logo',
          src: 'img/onion_logo_purple.svg',
          srcDark: 'img/onion_logo.svg',
        },
        items: [
          {
            type: 'docSidebar',
            sidebarId: 'tutorialSidebar',
            position: 'left',
            label: 'Docs',
          },
          { to: '/blog', label: 'Blog', position: 'left' },
          {
            href: 'https://github.com/OnionUI/Ports-Collection/blob/main/README.md',
            label: 'Ports',
            position: 'left',
          },
          {
            href: 'https://github.com/OnionUI/Themes/blob/main/README.md',
            label: 'Themes',
            position: 'left',
          },
          { to: '/about', label: 'About', position: 'left' },
          {
            type: 'search',
            position: 'right',
          },
          {
            type: 'docsVersionDropdown',
            position: 'right',
          },
          {
            href: 'https://github.com/OnionUI/Onion',
            label: 'GitHub',
            position: 'right',
          },
        ],
      },
      footer: {
        style: 'dark',
        links: [
          {
            title: 'Docs',
            items: [
              {
                label: 'Getting Started',
                to: '/docs',
              },
              {
                label: 'FAQ',
                to: '/docs/faq',
              },
              {
                label: 'Emulators',
                to: '/docs/emulators',
              },
            ],
          },
          {
            title: 'Community',
            items: [
              {
                label: 'GitHub Discussions',
                href: 'https://github.com/OnionUI/Onion/discussions',
              },
              {
                label: 'Discord',
                href: 'https://discord.gg/Jd2azKX',
              },
              {
                label: 'Reddit',
                href: 'https://reddit.com/r/miyoomini',
              },
              {
                label: 'Facebook',
                href: 'https://www.facebook.com/groups/1090249395102552',
              },
            ],
          },
          {
            title: 'Donate ❤️',
            items: [
              {
                label: 'Support us on Ko-fi',
                href: 'https://ko-fi.com/Aemiii91',
              },
              {
                label: 'GitHub Sponsors',
                href: 'https://github.com/sponsors/Aemiii91',
              },
            ],
          },
          {
            title: 'More',
            items: [
              {
                label: 'GitHub',
                href: 'https://github.com/OnionUI/Onion',
              },
              {
                href: 'https://www.youtube.com/@OnionUI',
                label: 'YouTube',
              },
              {
                href: 'https://github.com/OnionUI/Ports-Collection/blob/main/README.md',
                label: 'Ports',
              },
              {
                href: 'https://github.com/OnionUI/Themes/blob/main/README.md',
                label: 'Themes',
              },
            ],
          },
        ],
        copyright: `
          <span class="keep-together">Copyright © ${new Date().getFullYear()} The Onion UI Development Team.</span>
          <span class="keep-together">Built with <a href="https://docusaurus.io/" target="_blank">Docusaurus</a>.</span>
          <span class="keep-together"><a href="https://icons8.com/icons/stickers" target="_blank">Sticker icons</a> by <a href="https://icons8.com" target="_blank">Icons8</a>.</span>
        `,
      },
      prism: {
        theme: lightCodeTheme,
        darkTheme: darkCodeTheme,
        additionalLanguages: ['powershell', 'bash'],
      },
      colorMode: {
        defaultMode: 'dark',
        respectPrefersColorScheme: true,
      },
      zoom: {
        selector: '.markdown :not(em):not(h2):not(sup):not(a) > img, .markdown > img',
        config: {
          // options you can specify via https://github.com/francoischalifour/medium-zoom#usage
          background: {
            light: 'rgb(255, 255, 255)',
            dark: 'rgb(50, 50, 50)'
          }
        }
      },
    }),

  plugins: [
    require.resolve('docusaurus-plugin-image-zoom'),
    require.resolve('docusaurus-plugin-sass'),
    [
      "./src/plugins/blog-plugin",
      {
        id: "blog",
        routeBasePath: "blog",
        path: "./blog",
        truncateMarker: new RegExp("<Truncate \/>"),
        showReadingTime: false,
        remarkPlugins: [
          [oembed, oembedOpts],
          [remarkPing, remarkPingOpts],
        ],
      }
    ],
  ],
};

module.exports = config;
