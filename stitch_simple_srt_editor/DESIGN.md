---
name: Desktop Utility System
colors:
  surface: '#f9f9f9'
  surface-dim: '#dadada'
  surface-bright: '#f9f9f9'
  surface-container-lowest: '#ffffff'
  surface-container-low: '#f3f3f3'
  surface-container: '#eeeeee'
  surface-container-high: '#e8e8e8'
  surface-container-highest: '#e2e2e2'
  on-surface: '#1a1c1c'
  on-surface-variant: '#404752'
  inverse-surface: '#2f3131'
  inverse-on-surface: '#f1f1f1'
  outline: '#717783'
  outline-variant: '#c0c7d4'
  surface-tint: '#0060ab'
  primary: '#005faa'
  on-primary: '#ffffff'
  primary-container: '#0078d4'
  on-primary-container: '#ffffff'
  inverse-primary: '#a3c9ff'
  secondary: '#5e5e5e'
  on-secondary: '#ffffff'
  secondary-container: '#e4e2e2'
  on-secondary-container: '#646464'
  tertiary: '#974700'
  on-tertiary: '#ffffff'
  tertiary-container: '#bc5b00'
  on-tertiary-container: '#ffffff'
  error: '#ba1a1a'
  on-error: '#ffffff'
  error-container: '#ffdad6'
  on-error-container: '#93000a'
  primary-fixed: '#d3e3ff'
  primary-fixed-dim: '#a3c9ff'
  on-primary-fixed: '#001c39'
  on-primary-fixed-variant: '#004883'
  secondary-fixed: '#e4e2e2'
  secondary-fixed-dim: '#c8c6c6'
  on-secondary-fixed: '#1b1c1c'
  on-secondary-fixed-variant: '#474747'
  tertiary-fixed: '#ffdbc8'
  tertiary-fixed-dim: '#ffb689'
  on-tertiary-fixed: '#311300'
  on-tertiary-fixed-variant: '#743500'
  background: '#f9f9f9'
  on-background: '#1a1c1c'
  surface-variant: '#e2e2e2'
typography:
  menu-bar:
    fontFamily: Inter
    fontSize: 13px
    fontWeight: '400'
    lineHeight: 16px
  tool-label:
    fontFamily: Inter
    fontSize: 12px
    fontWeight: '400'
    lineHeight: 14px
  body-md:
    fontFamily: Inter
    fontSize: 14px
    fontWeight: '400'
    lineHeight: 20px
  data-grid:
    fontFamily: Inter
    fontSize: 13px
    fontWeight: '400'
    lineHeight: 18px
  header-sm:
    fontFamily: Inter
    fontSize: 12px
    fontWeight: '600'
    lineHeight: 16px
  mono-code:
    fontFamily: monospace
    fontSize: 13px
    fontWeight: '400'
    lineHeight: 18px
spacing:
  unit: 4px
  toolbar-height: 32px
  menu-height: 28px
  panel-padding: 8px
  gutter: 1px
  item-gap: 4px
---

## Brand & Style

This design system is built for high-productivity desktop software where reliability and performance are paramount. It draws inspiration from modern C++ application frameworks like Qt6, emphasizing a "native" feel that respects the conventions of desktop operating systems. 

The personality is functional, systematic, and utilitarian. It prioritizes information density and clear affordances over decorative flourishes. The design style is **Corporate / Modern** with a focus on precision: it uses a highly structured layout, subtle gradients to provide tactile cues for interactive elements, and a restrained color palette that directs attention toward user content rather than the UI frame. The goal is to evoke a sense of professional-grade stability and precision.

## Colors

The color palette is strictly neutral to support long-duration professional use without eye strain. 
- **Primary:** A classic Windows-inspired blue (#0078d4) used for primary actions, focus states, and progress indicators.
- **Backgrounds:** The main application window uses a light gray (#f0f0f0), while interactive workspaces and document areas use pure white (#ffffff) to provide maximum contrast for data.
- **Borders:** A consistent mid-gray (#c0c0c0) is used to define the boundaries of panels, toolbars, and input fields.
- **Semantic Colors:** Success, Warning, and Error states should use standard industrial tones (Green #107c10, Amber #ffb900, Red #a4262c).

## Typography

This design system uses **Inter** as the primary typeface due to its exceptional legibility in small-scale desktop interfaces and its neutral, systematic character. 

Typography is utilized to create hierarchy through weight and casing rather than large size variations. Most interface text stays between 12px and 14px to maintain high information density. Uppercase styling is reserved for panel headers and section labels to distinguish them from interactive content. In code editors or data tables, a monospaced font may be substituted for alignment precision.

## Layout & Spacing

The layout follows a **Fluid Grid** philosophy where components are contained within resizable panels. The structure is hierarchical:
1. **Top Tier:** Global Menu Bar followed by a fixed-height Toolbar.
2. **Middle Tier:** A multi-pane layout featuring a collapsible Sidebar (left/right) for properties/navigation and a central Workspace.
3. **Bottom Tier:** A thin Status Bar for state information.

Spacing follows a 4px base unit. Visual separation between panels is primarily achieved via 1px solid borders (#c0c0c0) rather than wide margins, maximizing the usable area for data and tools. Component internal padding is tight (8px) to accommodate complex utility workflows.

## Elevation & Depth

This design system avoids heavy shadows, instead using **Tonal Layers** and **Subtle Gradients** to communicate depth:
- **Level 0 (Background):** The #f0f0f0 frame.
- **Level 1 (Surface):** White (#ffffff) panels used for editors and data grids.
- **Interactive Elements:** Buttons and tabs feature a very subtle vertical linear gradient (top: #ffffff to bottom: #f5f5f5) to appear slightly raised. 
- **Inset Elements:** Input fields and data cells use a 1px inset border or "inner shadow" effect to appear recessed into the surface.
- **Pop-ups:** Context menus and tooltips use a small, 4px blur ambient shadow with 10% opacity to separate them from the main UI stack.

## Shapes

The shape language is defined by **Sharp Corners (0px)** or exceptionally small radii (max 2px) for specific buttons. This reinforces the technical, precise nature of the application. 

- **Containers:** All panels, windows, and data cells must have 0px corners.
- **Interactive Elements:** Standard buttons and dropdowns use a 2px radius to provide a slight visual soften that distinguishes them from structural containers.
- **Focus Indicators:** Use a 1px dashed or solid blue outline offset by 1px from the element's edge.

## Components

- **Buttons:** Rectangular with a 2px radius. Standard state uses a subtle gradient and 1px border. Hover state darkens the border; Active/Pressed state uses a flat, slightly darker background (#e5e5e5) with an inset appearance.
- **Data Grids:** High-density rows (24px-28px height). Alternating row colors (Zebra striping) using #f9f9f9. Headers are flat gray with vertical separators.
- **Toolbars:** Icons are typically 16x16 or 24x24 px. Flat styling until hovered, which reveals a light gray ghost-button background.
- **Input Fields:** White background with a #c0c0c0 border. On focus, the border changes to Primary Blue (#0078d4) with a 1px thickness.
- **Side Panels:** "Dockable" panels with a header bar containing the panel name in bold uppercase 12px text and a small "x" or "pin" icon.
- **Tabs:** Square, non-rounded tabs located at the top of panels. Active tabs have a white background that merges with the content area and a 2px blue accent line on the top edge.
- **Checkboxes/Radios:** Square (checkbox) or Circular (radio) with sharp 1px borders. Checked state uses the Primary Blue fill.