const MINIMUM_COLS = 2
const MINIMUM_ROWS = 1

class FitAddon {
  constructor() {}

  activate(terminal) {
    this._terminal = terminal
  }

  dispose() {}

  fit() {
    const dims = this.proposeDimensions()
    if (!dims || !this._terminal || isNaN(dims.cols) || isNaN(dims.rows)) {
      return
    }

    // TODO: Remove reliance on private API
    const core = this._terminal._core

    // Force a full render
    if (
      this._terminal.rows !== dims.rows ||
      this._terminal.cols !== dims.cols
    ) {
      core._renderService.clear()
      this._terminal.resize(dims.cols, dims.rows)
    }
  }

  proposeDimensions() {
    if (!this._terminal) {
      return undefined
    }

    if (!this._terminal.element || !this._terminal.element.parentElement) {
      return undefined
    }

    // TODO: Remove reliance on private API
    const core = this._terminal._core
    const dims = core._renderService.dimensions

    if (dims.actualCellWidth === 0 || dims.actualCellHeight === 0) {
      return undefined
    }

    const scrollbarWidth =
      this._terminal.options.scrollback === 0 ? 0 : core.viewport.scrollBarWidth

    const parentElementStyle = window.getComputedStyle(
      this._terminal.element.parentElement
    )
    const parentElementHeight = parseInt(
      parentElementStyle.getPropertyValue("height")
    )
    const parentElementWidth = Math.max(
      0,
      parseInt(parentElementStyle.getPropertyValue("width"))
    )
    const elementStyle = window.getComputedStyle(this._terminal.element)
    const elementPadding = {
      top: parseInt(elementStyle.getPropertyValue("padding-top")),
      bottom: parseInt(elementStyle.getPropertyValue("padding-bottom")),
      right: parseInt(elementStyle.getPropertyValue("padding-right")),
      left: parseInt(elementStyle.getPropertyValue("padding-left"))
    }
    const elementPaddingVer = elementPadding.top + elementPadding.bottom
    const elementPaddingHor = elementPadding.right + elementPadding.left
    const availableHeight = parentElementHeight - elementPaddingVer
    const availableWidth =
      parentElementWidth - elementPaddingHor - scrollbarWidth
    const geometry = {
      cols: Math.max(
        MINIMUM_COLS,
        Math.floor(availableWidth / dims.actualCellWidth)
      ),
      rows: Math.max(
        MINIMUM_ROWS,
        Math.floor(availableHeight / dims.actualCellHeight)
      )
    }
    return geometry
  }
}


const term = new Terminal(
  {
    cursorBlink: true,
    fontSize: 16,
    theme: {
      background: '#282C34',
      foreground: '#ffffff',
      cursor: '#ffffff',
      cursorAccent: '#282C34',
      selection: '#41454E',
    },
  }
);

var command = "";
var need_more_lines = false;
var stopped = false;
var repl = 0;

var Module = {
    'print': function(text) { 
      term.write(text + "\r\n");
    },
    'printErr': function(text) {
      term.write(text + "\r\n");
    },
    'onRuntimeInitialized': function(text) {
      var vm = Module.ccall('pkpy_new_vm', 'number', ['boolean'], [true]);
      repl = Module.ccall('pkpy_new_repl', 'number', ['number'], [vm]);
      term.write(need_more_lines ? "... " : ">>> ");
    },
    'onAbort': function(text) { 
      stopped = true;
    },
  };

function term_init() {
    term.open(document.getElementById('terminal'));
    const addon = new FitAddon();
    term.loadAddon(addon);
    addon.fit();

    // refit when window is resized
    window.addEventListener('resize', () => {
      addon.fit();
    });

    term.onData(e => {
      if (stopped) return;
      switch (e) {
        case '\r': // Enter
          term.write("\r\n");
          need_more_lines = Module.ccall('pkpy_repl_input', 'bool', ['number', 'string'], [repl, command]);
          command = '';
          term.write(need_more_lines ? "... " : ">>> ");
          break;
        case '\u007F': // Backspace (DEL)
          var cnt = term._core.buffer.x-4;
          if(cnt<=0 || command.length==0) break;
          // delete the last unicode char
          command = command.replace(/.$/u, "");
          // clear the whole line
          term.write('\b \b'.repeat(cnt));
          // re-write the command
          term.write(command);
          break;
        default: // Print all other characters for demo
          if (e >= String.fromCharCode(0x20) && e <= String.fromCharCode(0x7E) || e >= '\u00a0') {
            command += e;
            term.write(e);
          }
      }
    });
}
