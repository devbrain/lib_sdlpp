import React from 'react';
import clsx from 'clsx';
import Link from '@docusaurus/Link';
import useDocusaurusContext from '@docusaurus/useDocusaurusContext';
import Layout from '@theme/Layout';
import HomepageFeatures from '@site/src/components/HomepageFeatures';
import CodeBlock from '@theme/CodeBlock';

import styles from './index.module.css';

function HomepageHeader() {
  const {siteConfig} = useDocusaurusContext();
  return (
    <header className={clsx('hero hero--primary', styles.heroBanner)}>
      <div className="container">
        <h1 className="hero__title">{siteConfig.title}</h1>
        <p className="hero__subtitle">{siteConfig.tagline}</p>
        <div className={styles.buttons}>
          <Link
            className="button button--secondary button--lg"
            to="/docs/intro">
            Get Started - 5min ⏱️
          </Link>
        </div>
      </div>
    </header>
  );
}

function QuickExample() {
  const codeExample = `#include <sdlpp/core/core.hh>
#include <sdlpp/video/window.hh>
#include <sdlpp/video/renderer.hh>
#include <sdlpp/events/events.hh>

int main() {
    // Initialize SDL with RAII
    auto init = sdlpp::init::create(sdlpp::init_flags::video);
    if (!init) {
        std::cerr << "Failed to initialize: " << init.error() << "\\n";
        return 1;
    }
    
    // Create window and renderer
    auto window = sdlpp::window::create("SDL++ Demo", 800, 600);
    auto renderer = sdlpp::renderer::create(*window);
    
    // Main loop
    bool running = true;
    while (running) {
        // Handle events
        while (auto event = sdlpp::event_queue::poll()) {
            if (event->type() == sdlpp::event_type::quit) {
                running = false;
            }
        }
        
        // Render
        renderer->set_draw_color({0, 128, 255, 255});
        renderer->clear();
        renderer->present();
    }
    
    return 0;
}`;

  return (
    <section className={styles.quickExample}>
      <div className="container">
        <h2>Simple, Safe, Modern</h2>
        <p>SDL++ wraps SDL3's power in modern C++20 with RAII, type safety, and zero-cost abstractions.</p>
        <CodeBlock language="cpp" title="main.cpp">
          {codeExample}
        </CodeBlock>
      </div>
    </section>
  );
}

export default function Home() {
  const {siteConfig} = useDocusaurusContext();
  return (
    <Layout
      title={`Modern C++20 Wrapper for SDL3`}
      description="SDL++ is a comprehensive, type-safe C++20 wrapper library for SDL3. RAII resource management, modern C++ idioms, and zero-cost abstractions.">
      <HomepageHeader />
      <QuickExample />
      <main>
        <HomepageFeatures />
      </main>
    </Layout>
  );
}