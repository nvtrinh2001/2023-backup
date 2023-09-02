return {
  {
    "folke/styler.nvim",
    event = "VeryLazy",
    config = function()
      require("styler").setup {
        themes = {
          markdown = { colorscheme = "gruvbox" },
          help = { colorscheme = "gruvbox" },
        },
      }
    end,
  },
  {
    "catppuccin/nvim",
    lazy = false,
    priority = 1000,
    opts = {
      transparent_background = true
    },
    config = function(_, opts)
      local catppuccin = require "catppuccin"
      catppuccin.setup(opts)
      catppuccin.load()
    end,
  },
  { "rebelot/kanagawa.nvim", lazy = false, name = "kanagawa" },
  {
    "ellisonleao/gruvbox.nvim",
    lazy = false,
    config = function()
      require("gruvbox").setup()
    end,
  },
}
