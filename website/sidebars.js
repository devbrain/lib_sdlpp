/**
 * Creating a sidebar enables you to:
 - create an ordered group of docs
 - render a sidebar for each doc of that group
 - provide next/previous navigation

 The sidebars can be generated from the filesystem, or explicitly defined here.

 Create as many sidebars as you want.
 */

// @ts-check

/** @type {import('@docusaurus/plugin-content-docs').SidebarsConfig} */
const sidebars = {
  // Tutorial/Getting Started sidebar
  tutorialSidebar: [
    'intro',
    {
      type: 'category',
      label: 'Getting Started',
      items: [
        'getting-started/installation',
        'getting-started/quick-start',
        'getting-started/migration-guide',
      ],
    },
  ],

  // API Reference sidebar
  apiSidebar: [
    'api/overview',
    {
      type: 'category',
      label: 'Concepts',
      items: [
        'concepts/index',
        'concepts/geometry-concepts',
        'concepts/renderer-concepts',
      ],
    },
    {
      type: 'category',
      label: 'Core',
      items: [
        'api/core/initialization',
        'api/core/error',
        'api/core/timer',
        'api/core/version',
      ],
    },
    {
      type: 'category',
      label: 'Application',
      items: [
        'api/app/application-framework',
      ],
    },
    {
      type: 'category',
      label: 'Video',
      items: [
        'api/video/window',
        'api/video/renderer',
        'api/video/surface',
        'api/video/surface-renderer',
        'api/video/texture',
        'api/video/gl',
      ],
    },
    {
      type: 'category',
      label: 'Audio',
      items: [
        'api/audio/audio-playback',
      ],
    },
    {
      type: 'category',
      label: 'Events',
      items: [
        'api/events/event-queue',
        'api/events/event-types',
      ],
    },
    {
      type: 'category',
      label: 'Input',
      items: [
        'api/input/keyboard',
        'api/input/mouse',
        'api/input/gamepad',
      ],
    },
    {
      type: 'category',
      label: 'Config',
      items: [
        'api/config/hints',
        'api/config/properties',
      ],
    },
    {
      type: 'category',
      label: 'UI',
      items: [
        'api/ui/message-box',
        'api/ui/clipboard',
      ],
    },
    {
      type: 'category',
      label: 'Utility',
      items: [
        'api/utility/geometry',
      ],
    },
    {
      type: 'category',
      label: 'System',
      items: [
        'api/system/filesystem',
        'api/system/platform',
        'api/system/cpu-info',
        'api/system/process',
        'api/system/locale',
      ],
    },
    {
      type: 'category',
      label: 'I/O',
      items: [
        'api/io/overview',
        'api/io/iostream',
        'api/io/async-io',
        'api/io/storage',
      ],
    },
  ],

  // Examples sidebar
  examplesSidebar: [
    'examples/index',
    {
      type: 'category',
      label: 'Getting Started',
      items: [
        'examples/hello-window',
        'examples/drawing-shapes',
      ],
    },
  ],
};

module.exports = sidebars;