# 🎨 Visual Guide

## Interface Layout

```
┌─────────────────────────────────────────────────────────────────┐
│  🔧 Array Manipulation Compiler                                 │
│  LR Parser with Semantic Analysis & Code Generation             │
├─────────────────────────────────┬───────────────────────────────┤
│                                 │                               │
│  Source Code                    │  🌳 AST  📊 Symbols  ⚙️ TAC   │
│  ┌─────────────────────────┐   │  ┌───────────────────────┐   │
│  │ 1  int x[2][2] = {      │   │  │ ┌─ Program            │   │
│  │ 2    {1,2},             │   │  │ │ └─ DeclAssign       │   │
│  │ 3    {3,4}              │   │  │ │   ├─ int            │   │
│  │ 4  };                   │   │  │ │   ├─ ID: x          │   │
│  │ 5                       │   │  │ │   ├─ Dimensions     │   │
│  │                         │   │  │ │   │  ├─ NUM: 2      │   │
│  │                         │   │  │ │   │  └─ NUM: 2      │   │
│  │                         │   │  │ │   └─ Array          │   │
│  │                         │   │  │ │     ├─ ArrayRow     │   │
│  │                         │   │  │ │     └─ ArrayRow     │   │
│  │                         │   │  │ └─────────────────────│   │
│  └─────────────────────────┘   │  └───────────────────────┘   │
│                                 │                               │
│  [▶️ Compile & Run]             │  [Tree View] [JSON View]     │
│                                 │                               │
└─────────────────────────────────┴───────────────────────────────┘
```

## Color Scheme

### Editor Colors
```
Background:     #1e1e1e  ████████
Panel:          #252526  ████████
Border:         #3e3e42  ████████
Text:           #d4d4d4  ████████
```

### Syntax Highlighting
```
Keywords:       #569cd6  ████████  (int, float)
Strings:        #ce9178  ████████  ("text")
Numbers:        #b5cea8  ████████  (123, 4.5)
Comments:       #6a9955  ████████  (// comment)
Functions:      #dcdcaa  ████████  (function())
Types:          #4ec9b0  ████████  (int, float)
```

### UI Accents
```
Primary:        #667eea  ████████  (buttons, active)
Secondary:      #764ba2  ████████  (gradients)
Success:        #4ec9b0  ████████  (badges)
Error:          #f48771  ████████  (errors)
Warning:        #dcdcaa  ████████  (warnings)
```

## Component Previews

### 1. Code Editor
```
┌─────────────────────────────────────┐
│ Source Code          [▶️ Compile]   │
├─────────────────────────────────────┤
│  1  int x[2][2] = {{1,2},{3,4}};   │
│  2                                  │
│  3                                  │
│  4                                  │
│  5                                  │
│                                     │
│  Monaco Editor                      │
│  • Syntax highlighting              │
│  • Line numbers                     │
│  • Auto-indent                      │
│  • Find & replace                   │
└─────────────────────────────────────┘
```

### 2. AST Visualizer (Tree View)
```
┌─────────────────────────────────────┐
│ [Tree View] [JSON View]             │
├─────────────────────────────────────┤
│ ▼ Program                           │
│   └─ ▼ DeclAssign                   │
│       ├─ ● int = int                │
│       ├─ ● ID = x                   │
│       ├─ ▼ Dimensions               │
│       │   ├─ ● NUM = 2              │
│       │   └─ ● NUM = 2              │
│       └─ ▼ Array                    │
│           ├─ ▼ ArrayRow             │
│           │   ├─ ● NUM = 1          │
│           │   └─ ● NUM = 2          │
│           └─ ▼ ArrayRow             │
│               ├─ ● NUM = 3          │
│               └─ ● NUM = 4          │
└─────────────────────────────────────┘
```

### 3. Symbol Table
```
┌─────────────────────────────────────┐
│ Symbol Table                        │
├────────┬──────┬───────┬─────────────┤
│ Symbol │ Type │ Array │ Dimensions  │
├────────┼──────┼───────┼─────────────┤
│   x    │ int  │  Yes  │ [2] [2]     │
│   y    │ int  │  No   │     -       │
│  arr   │ float│  Yes  │ [5]         │
└────────┴──────┴───────┴─────────────┘

Symbol Details:
{
  "x": {
    "type": "int",
    "isArray": true,
    "size1": 2,
    "size2": 2
  }
}
```

### 4. TAC View
```
┌─────────────────────────────────────┐
│ Three-Address Code    8 instructions│
├─────────────────────────────────────┤
│  1  // DECL x  type=int             │
│  2  t1 = 0 * 4                      │
│  3  x[t1] = 1                       │
│  4  t2 = 1 * 4                      │
│  5  x[t2] = 2                       │
│  6  t3 = 2 * 4                      │
│  7  x[t3] = 3                       │
│  8  t4 = 3 * 4                      │
│  9  x[t4] = 4                       │
└─────────────────────────────────────┘
```

## Button States

### Compile Button
```
Normal:    [▶️ Compile & Run]  (Purple gradient)
Hover:     [▶️ Compile & Run]  (Lifted, brighter)
Active:    [▶️ Compile & Run]  (Pressed down)
Loading:   [⏳ Compiling...]   (Disabled, dimmed)
```

### Tab Buttons
```
Inactive:  [ AST ]  (Gray background)
Hover:     [ AST ]  (Light gray)
Active:    [ AST ]  (Purple, underline)
```

## Loading States

### Compilation in Progress
```
┌─────────────────────────────────────┐
│                                     │
│         ⏳                          │
│                                     │
│    Compiling your code...           │
│                                     │
│         [Spinner]                   │
│                                     │
└─────────────────────────────────────┘
```

### Empty State
```
┌─────────────────────────────────────┐
│                                     │
│         📝                          │
│                                     │
│      Ready to Compile               │
│                                     │
│  Write your code and click          │
│  "Compile & Run" to see results     │
│                                     │
│  Example:                           │
│  ┌─────────────────────────────┐   │
│  │ int x[2][2] = {{1,2},{3,4}};│   │
│  └─────────────────────────────┘   │
└─────────────────────────────────────┘
```

### Error State
```
┌─────────────────────────────────────┐
│                                     │
│         ❌                          │
│                                     │
│    Compilation Error                │
│                                     │
│  ┌─────────────────────────────┐   │
│  │ syntax error at line 1:     │   │
│  │ unexpected token '{'        │   │
│  └─────────────────────────────┘   │
│                                     │
└─────────────────────────────────────┘
```

## Responsive Behavior

### Desktop (> 1200px)
```
┌────────────────────────────────────────────┐
│  Header                                    │
├──────────────────┬─────────────────────────┤
│                  │                         │
│   Editor (40%)   │   Output (60%)          │
│                  │                         │
└──────────────────┴─────────────────────────┘
```

### Tablet (768px - 1200px)
```
┌────────────────────────────────────────────┐
│  Header                                    │
├──────────────────┬─────────────────────────┤
│                  │                         │
│   Editor (50%)   │   Output (50%)          │
│                  │                         │
└──────────────────┴─────────────────────────┘
```

### Mobile (< 768px)
```
┌────────────────────────────────────────────┐
│  Header                                    │
├────────────────────────────────────────────┤
│                                            │
│   Editor (100%)                            │
│                                            │
├────────────────────────────────────────────┤
│                                            │
│   Output (100%)                            │
│                                            │
└────────────────────────────────────────────┘
```

## Interaction Patterns

### Tree Node Expansion
```
Before Click:
▶ DeclAssign

After Click:
▼ DeclAssign
  ├─ int
  ├─ ID: x
  └─ ...
```

### Tab Switching
```
Click "Symbol Table" tab:
1. Tab becomes active (purple)
2. Content fades out (100ms)
3. New content fades in (100ms)
4. Smooth transition
```

### Hover Effects
```
Tree Node:
  Normal:  background: transparent
  Hover:   background: rgba(255,255,255,0.05)

Table Row:
  Normal:  background: transparent
  Hover:   background: rgba(255,255,255,0.05)

Button:
  Normal:  transform: translateY(0)
  Hover:   transform: translateY(-2px)
```

## Typography

### Font Families
```
UI Text:     -apple-system, BlinkMacSystemFont, 'Segoe UI'
Code:        'Courier New', monospace
Editor:      Monaco, Consolas, 'Courier New'
```

### Font Sizes
```
Header Title:      1.8rem (28.8px)
Section Header:    1.1rem (17.6px)
Body Text:         0.95rem (15.2px)
Code:              0.9rem (14.4px)
Small Text:        0.85rem (13.6px)
```

### Font Weights
```
Bold:       700
Semibold:   600
Medium:     500
Regular:    400
Light:      300
```

## Spacing System

```
XXS:  0.25rem (4px)
XS:   0.5rem  (8px)
SM:   0.75rem (12px)
MD:   1rem    (16px)
LG:   1.5rem  (24px)
XL:   2rem    (32px)
XXL:  3rem    (48px)
```

## Border Radius

```
Small:   3px   (badges, tags)
Medium:  4px   (buttons, inputs)
Large:   6px   (panels, cards)
XLarge:  8px   (containers)
Round:   50%   (circles)
```

## Shadows

```
Small:   0 2px 4px rgba(0,0,0,0.1)
Medium:  0 2px 8px rgba(0,0,0,0.2)
Large:   0 4px 12px rgba(0,0,0,0.3)
Button:  0 2px 8px rgba(102,126,234,0.3)
```

## Animations

### Durations
```
Fast:    100ms  (hover, active)
Normal:  200ms  (transitions)
Slow:    300ms  (complex animations)
```

### Easing
```
Standard:  ease
In:        ease-in
Out:       ease-out
InOut:     ease-in-out
```

## Accessibility

### Focus States
```
Keyboard Focus:
  outline: 2px solid #667eea
  outline-offset: 2px
```

### Color Contrast
```
Text on Dark:     #d4d4d4 on #1e1e1e  ✅ WCAG AA
Buttons:          #ffffff on #667eea  ✅ WCAG AAA
Links:            #667eea on #1e1e1e  ✅ WCAG AA
```

### Screen Reader Support
```
- Semantic HTML
- ARIA labels where needed
- Keyboard navigation
- Focus management
```

## Print Styles

```
@media print {
  - Hide navigation
  - Expand all tree nodes
  - Black text on white
  - Page breaks at sections
}
```

---

This visual guide helps you understand the UI design and implementation details.
