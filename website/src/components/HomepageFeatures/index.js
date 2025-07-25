import React from 'react';
import clsx from 'clsx';
import styles from './styles.module.css';

const FeatureList = [
  {
    title: 'Type-Safe & Modern',
    icon: '🛡️',
    description: (
      <>
        Built with C++20 features including concepts, designated initializers,
        and ranges. Strong typing prevents common SDL mistakes at compile time.
      </>
    ),
  },
  {
    title: 'RAII Resource Management',
    icon: '♻️',
    description: (
      <>
        All SDL resources are automatically managed. No manual cleanup needed -
        resources are freed when they go out of scope.
      </>
    ),
  },
  {
    title: 'Zero-Cost Abstractions',
    icon: '⚡',
    description: (
      <>
        Template-based design ensures no runtime overhead. Get the safety and
        convenience of C++ without sacrificing performance.
      </>
    ),
  },
  {
    title: 'Comprehensive Coverage',
    icon: '🎯',
    description: (
      <>
        Wraps most SDL3 functionality including video, audio, input devices,
        events, filesystem operations, and platform-specific features.
      </>
    ),
  },
  {
    title: 'Error Handling with Expected',
    icon: '✅',
    description: (
      <>
        Uses std::expected for error handling. No exceptions for SDL errors -
        clear error propagation with monadic operations.
      </>
    ),
  },
  {
    title: 'Extensible Design',
    icon: '🔧',
    description: (
      <>
        Geometry concepts allow integration with your math library. Flexible
        design lets you extend SDL++ for your specific needs.
      </>
    ),
  },
];

function Feature({icon, title, description}) {
  return (
    <div className={clsx('col col--4')}>
      <div className="text--center">
        <div className={styles.featureIcon}>{icon}</div>
      </div>
      <div className="text--center padding-horiz--md">
        <h3>{title}</h3>
        <p>{description}</p>
      </div>
    </div>
  );
}

export default function HomepageFeatures() {
  return (
    <section className={styles.features}>
      <div className="container">
        <div className="row">
          {FeatureList.map((props, idx) => (
            <Feature key={idx} {...props} />
          ))}
        </div>
      </div>
    </section>
  );
}