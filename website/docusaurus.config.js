// @ts-check
// Note: type annotations allow type checking and IDEs autocompletion

const lightCodeTheme = require('prism-react-renderer').themes.github;
const darkCodeTheme = require('prism-react-renderer').themes.dracula;

/** @type {import('@docusaurus/types').Config} */
const config = {
  title: 'SDL++',
  tagline: 'Modern C++20 Wrapper for SDL3',
  favicon: 'img/favicon.ico',

  // Set the production url of your site here
  url: 'https://sdlpp.dev',
  // Set the /<baseUrl>/ pathname under which your site is served
  baseUrl: '/',

  // GitHub pages deployment config.
  organizationName: 'sdlpp', // Usually your GitHub org/user name.
  projectName: 'sdlpp', // Usually your repo name.

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
          // Please change this to your repo.
          editUrl: 'https://github.com/sdlpp/sdlpp/tree/main/website/',
        },
        blog: {
          showReadingTime: true,
          // Please change this to your repo.
          editUrl: 'https://github.com/sdlpp/sdlpp/tree/main/website/',
        },
        theme: {
          customCss: require.resolve('./src/css/custom.css'),
        },
      }),
    ],
  ],

  themeConfig:
    /** @type {import('@docusaurus/preset-classic').ThemeConfig} */
    ({
      // Replace with your project's social card
      image: 'img/sdlpp-social-card.jpg',
      navbar: {
        title: 'SDL++',
        logo: {
          alt: 'SDL++ Logo',
          src: 'img/logo.svg',
        },
        items: [
          {
            type: 'docSidebar',
            sidebarId: 'tutorialSidebar',
            position: 'left',
            label: 'Documentation',
          },
          {
            type: 'docSidebar',
            sidebarId: 'apiSidebar',
            position: 'left',
            label: 'API Reference',
          },
          {
            type: 'docSidebar',
            sidebarId: 'examplesSidebar',
            position: 'left',
            label: 'Examples',
          },
          {to: '/blog', label: 'Blog', position: 'left'},
          {
            href: 'https://github.com/sdlpp/sdlpp',
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
                to: '/docs/intro',
              },
              {
                label: 'API Reference',
                to: '/docs/api/overview',
              },
              {
                label: 'Examples',
                to: '/docs/examples/',
              },
            ],
          },
          {
            title: 'Community',
            items: [
              {
                label: 'Stack Overflow',
                href: 'https://stackoverflow.com/questions/tagged/sdlpp',
              },
              {
                label: 'Discord',
                href: 'https://discord.gg/sdlpp',
              },
              {
                label: 'Twitter',
                href: 'https://twitter.com/sdlpp',
              },
            ],
          },
          {
            title: 'More',
            items: [
              {
                label: 'Blog',
                to: '/blog',
              },
              {
                label: 'GitHub',
                href: 'https://github.com/sdlpp/sdlpp',
              },
              {
                label: 'SDL3 Documentation',
                href: 'https://wiki.libsdl.org/SDL3/',
              },
            ],
          },
        ],
        copyright: `Copyright © ${new Date().getFullYear()} SDL++ Project. Built with Docusaurus.`,
      },
      prism: {
        theme: lightCodeTheme,
        darkTheme: darkCodeTheme,
        additionalLanguages: ['cpp', 'cmake', 'bash'],
      },
      algolia: {
        // The application ID provided by Algolia
        appId: 'YOUR_APP_ID',
        // Public API key: it is safe to commit it
        apiKey: 'YOUR_SEARCH_API_KEY',
        indexName: 'sdlpp',
        // Optional: see doc section below
        contextualSearch: true,
        // Optional: Algolia search parameters
        searchParameters: {},
        // Optional: path for search page that enabled by default (`false` to disable it)
        searchPagePath: 'search',
      },
    }),
};

module.exports = config;
