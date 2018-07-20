/*
MIT License

Copyright (c) 2018 Brian T. Park

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef ACE_ROUTINE_COMMAND_DISPATCHER_H
#define ACE_ROUTINE_COMMAND_DISPATCHER_H

#include <Arduino.h> // Serial
#include <AceRoutine.h>
#include "SerialReader.h"

namespace ace_routine {
namespace cli {

/**
 * Abstract class for command handlers.
 */
class CommandHandler {
  public:
    /**
     * Constructor.
     *
     * @param helpString The "usage" string, excluding the name of the command
     *        itself to save space.
     * @param name Name of the command will be automatically added by the
     *        'help' handler.
     */
    CommandHandler(const char* name, const char* helpString):
        mName(name), mHelpString(helpString) {}

    virtual void run(int argc, const char** argv) const = 0;

    const char* getName() const { return mName; }

    const char* getHelpString() const { return mHelpString; }

  private:
    const char* const mName;
    const char* const mHelpString;
};

/**
 * A coroutine that reads lines from the Serial port, tokenizes the line on
 * whitespace boundaries, and calls the appropriate command handler to handle
 * the command. Command have the form "command arg1 arg2 ...", where the 'arg*'
 * can be any string.
 *
 * The calling code is expected to provide a mapping of the command string
 * (e.g. "list") to its command handler (CommandHandler). The CommandHandler is
 * called with the number of arguments (argc) and the array of tokens (argv),
 * just like the arguments of the C-language main() function.
 */
class CommandDispatcher: public Coroutine {
  public:
    /** Maximum number of tokens for a command including flags. */
    static const uint8_t ARGV_SIZE = 10;

    /**
     * Constructor.
     *
     * @param serialReader Instance of SerialReader.
     * @param numCommands Number of entries in handlers.
     * @param handlers An array of CommandHandler pointers.
     */
    CommandDispatcher(
            SerialReader& serialReader,
            uint8_t numCommands,
            const CommandHandler* const* handlers):
        mSerialReader(serialReader),
        mNumCommands(numCommands),
        mComandHandlers(handlers) {}

  private:
    static const uint8_t STATUS_SUCCESS = 0;
    static const uint8_t STATUS_BUFFER_OVERFLOW = 1;
    static const uint8_t STATUS_FLUSH_TO_EOL = 2;
    static const char DELIMS[];

    /** Print the error caused by the given line. */
    void printLineError(const char* line, uint8_t statusCode);

    /** Handle the 'help' command. */
    void helpCommandHandler(int argc, const char** argv);

    /** Tokenize the given line and run the command handler. */
    void runCommand(char* line);

    /** Tokenize the line, returning the number of tokens. */
    int tokenize(char* line, int argvSize, const char** argv);

    virtual int runRoutine() override;

    SerialReader& mSerialReader;
    const uint8_t mNumCommands;
    const CommandHandler* const* const mComandHandlers;
};

}
}

#endif
