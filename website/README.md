# SDL++ Documentation Website

This directory contains the Docusaurus-based documentation website for SDL++.

## Development

### Prerequisites

- Node.js 18.0 or above
- npm or yarn

### Installation

```bash
npm install
```

### Local Development

```bash
npm start
```

This command starts a local development server and opens up a browser window. Most changes are reflected live without having to restart the server.

### Build

```bash
npm run build
```

This command generates static content into the `build` directory and can be served using any static contents hosting service.

### Deployment

```bash
GIT_USER=<Your GitHub username> npm run deploy
```

If you are using GitHub pages for hosting, this command is a convenient way to build the website and push to the `gh-pages` branch.

## Structure

```
website/
├── docs/                   # Documentation files
│   ├── intro.md           # Landing page
│   ├── getting-started/   # Installation and setup guides
│   ├── api/              # API reference documentation
│   └── examples/         # Code examples
├── src/                   # React components and pages
│   ├── components/       # Reusable components
│   ├── pages/           # Custom pages
│   └── css/             # Custom styles
├── static/               # Static assets
├── docusaurus.config.js  # Docusaurus configuration
└── sidebars.js          # Sidebar configuration
```

## Contributing

### Adding Documentation

1. Create a new markdown file in the appropriate directory under `docs/`
2. Add front matter with sidebar position:
   ```yaml
   ---
   sidebar_position: 1
   ---
   ```
3. Write your documentation using Markdown
4. Update `sidebars.js` if needed

### Adding Examples

1. Create a new markdown file in `docs/examples/`
2. Include complete, runnable code examples
3. Add explanations and exercises
4. Update the examples sidebar in `sidebars.js`

### Styling

Custom styles are in `src/css/custom.css`. The site uses the Docusaurus classic theme with customizations for SDL++ branding.

## Writing Guidelines

- Use clear, concise language
- Include code examples for all features
- Provide both basic and advanced usage patterns
- Link to related documentation
- Test all code examples

## Building for Production

```bash
# Build the site
npm run build

# Test the build locally
npm run serve
```

## Troubleshooting

### Clear Cache

If you encounter build issues:

```bash
npm run clear
npm install
npm start
```

### Check for Broken Links

```bash
npm run build
# The build will fail if there are broken links
```

## License

The documentation is licensed under the same license as SDL++ (see main project LICENSE file).