const { applyAcrylicEffect } = require("../index");

const hwnd = 1508708; // Replace with actual window handle

try {
  const result = applyAcrylicEffect({
    hwnd: hwnd,
    type: "acrylic",
    corner: "round",
    opacity: 10,
    tintColor: "000000",
    borderColor: "000000",
    borderVisible: true,
  });
  console.log("Individual test result:", result);
} catch (error) {
  console.error("Individual test error:", error.message);
}
