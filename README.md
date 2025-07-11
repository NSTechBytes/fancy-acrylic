# fancy-acrylic

A Node.js native addon for applying acrylic and blur effects to Windows windows using window handles.

## Features

- Apply acrylic or blur effects to any window using its handle (HWND)
- Customize corner styles (none, round, roundsmall)
- Set opacity and tint colors
- Set border colors on Windows 11+
- Cross-platform compatibility (Windows only for effects)

## Installation

```bash
npm install
npm run build
```

## Requirements

- Windows 10 or later
- Node.js with native module support
- Visual Studio Build Tools (for compilation)

## Usage

```javascript
const { applyAcrylicEffect, isSupported, getWindowsBuildNumber } = require('./index.js');

// Check if acrylic effects are supported
console.log('Supported:', isSupported());

// Get Windows build number
console.log('Build:', getWindowsBuildNumber());

// Apply acrylic effect
const result = applyAcrylicEffect({
    hwnd: 0x12345678, // Window handle as number
    type: 'acrylic',  // 'blur' or 'acrylic'
    corner: 'round',  // 'none', 'round', or 'roundsmall'
    opacity: 200,     // 0-255
    tintColor: 'FF0000',    // Optional: hex color (RRGGBB)
    borderColor: '00FF00'   // Optional: hex color (RRGGBB)
});
```

## API

### `applyAcrylicEffect(options)`

Applies acrylic or blur effect to a window.

**Parameters:**
- `options` (Object): Configuration options
  - `hwnd` (number): Window handle (HWND as number) - **required**
  - `type` (string): Effect type - `'blur'` or `'acrylic'` (default: `'acrylic'`)
  - `corner` (string): Corner style - `'none'`, `'round'`, or `'roundsmall'` (default: `'none'`)
  - `opacity` (number): Opacity value 0-255 (default: 204)
  - `tintColor` (string): Tint color in hex format (RRGGBB) - optional
  - `borderColor` (string): Border color in hex format (RRGGBB) - optional

**Returns:** `boolean` - `true` if successful

**Example:**
```javascript
applyAcrylicEffect({
    hwnd: windowHandle,
    type: 'acrylic',
    corner: 'round',
    opacity: 180,
    tintColor: '0078D4',
    borderColor: 'FF6B6B'
});
```

### `isSupported()`

Checks if acrylic effects are supported on the current system.

**Returns:** `boolean` - `true` if supported

### `getWindowsBuildNumber()`

Gets the current Windows build number.

**Returns:** `number` - Windows build number, or 0 if not Windows

## Getting Window Handles

To use this addon, you need to obtain window handles (HWND). You can:

1. Use other native modules that provide window enumeration
2. Use the Windows API from other languages
3. Use tools like `FindWindow` or `EnumWindows` from the Windows API

## Notes

- **tintColor and borderColor are optional**: If not provided, no color will be applied
- **Border is always visible**: The addon doesn't provide functionality to hide borders
- **Windows 11+ for border colors**: Border color setting only works on Windows 11 and later
- **Windows 10+ required**: Acrylic effects require Windows 10 or later

## Error Handling

The addon throws JavaScript errors for:
- Invalid parameters
- Invalid window handles
- Unsupported operations
- System API failures

## Building

```bash
# Install dependencies
npm install

# Build the native addon
npm run build

# Or use node-gyp directly
npx node-gyp configure
npx node-gyp build
```

## License

MIT License