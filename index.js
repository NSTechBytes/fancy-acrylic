const addon = require('./build/Release/fancy-acrylic');

/**
 * Apply acrylic or blur effect to a window
 * @param {Object} options - Configuration options
 * @param {number} options.hwnd - Window handle (HWND as number)
 * @param {string} [options.type='acrylic'] - Effect type: 'blur' or 'acrylic'
 * @param {string} [options.corner='none'] - Corner style: 'none', 'round', or 'roundsmall'
 * @param {number} [options.opacity=204] - Opacity value (0-255)
 * @param {string} [options.tintColor] - Tint color in hex format (RRGGBB)
 * @param {string} [options.borderColor] - Border color in hex format (RRGGBB)
 * @param {boolean} [options.borderVisible=true] - Whether border should be visible
 * @returns {boolean} True if successful
 */
function applyAcrylicEffect(options) {
    if (!options || typeof options !== 'object') {
        throw new Error('Options object is required');
    }
    
    if (typeof options.hwnd !== 'number') {
        throw new Error('hwnd must be a number');
    }
    
    return addon.applyAcrylicEffect(options);
}

/**
 * Check if acrylic effects are supported on this system
 * @returns {boolean} True if supported
 */
function isSupported() {
    return addon.isSupported();
}

/**
 * Get the current Windows build number
 * @returns {number} Windows build number, or 0 if not Windows
 */
function getWindowsBuildNumber() {
    return addon.getWindowsBuildNumber();
}

module.exports = {
    applyAcrylicEffect,
    isSupported,
    getWindowsBuildNumber
};

// Example usage:
if (require.main === module) {
    const hwnd = 1377636; // Replace with actual window handle

    try {
        const result = applyAcrylicEffect({
            hwnd: hwnd,
            type: "acrylic",
            corner: "round",
            opacity: 200,       
            tintColor: "ffffff",
            borderColor: "000000",
            borderVisible: false   
        });
        console.log("Individual test result:", result);
    } catch (error) {
        console.error("Individual test error:", error.message);
    }
}