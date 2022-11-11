const term = new Terminal();

var command = "";
var need_more_lines = false;
var stopped = false;

var Module = {
    'print': function(text) { 
      term.write(text + "\r\n");
    },
    'printErr': function(text) {
      term.write(text + "\r\n");
    },
    'onRuntimeInitialized': function(text) { 
      Module.ccall('repl_start', null, [], []);
      term.write(need_more_lines ? "... " : ">>> ");
    },
    'onAbort': function(text) { 
      stopped = true;
      term.write("Aborted\r\n");
    },
  };

function term_init() {
    term.open(document.getElementById('terminal'));
    term.onData(e => {
      if (stopped) return;
      switch (e) {
        case '\r': // Enter
          term.write("\r\n");
          if(command == 'exit()'){
            stopped = true;
            term.write("Bye!\r\n");
            break;
          }
          need_more_lines = Module.ccall('repl_input', Boolean, ['string'], [command]);
          command = '';
          term.write(need_more_lines ? "... " : ">>> ");
          break;
        case '\u007F': // Backspace (DEL)
          // Do not delete the prompt
          if (term._core.buffer.x > 4) {    // '>>> ' or '... '
            term.write('\b \b');
            if (command.length > 0) {
              command = command.substr(0, command.length - 1);
            }
          }
          break;
        default: // Print all other characters for demo
          if (e >= String.fromCharCode(0x20) && e <= String.fromCharCode(0x7E) || e >= '\u00a0') {
            command += e;
            term.write(e);
          }
      }
    });
}