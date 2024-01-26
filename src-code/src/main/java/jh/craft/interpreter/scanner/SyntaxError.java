package jh.craft.interpreter.scanner;

/**
 *
 * @param line the line where the error happened
 * @param position a valid index of source where the error happened
 * @param source the full text file that was being parsed
 * @param msg the error message
 */
public record SyntaxError(int line, int position, String source, String msg) {

    @Override
    public String toString(){
        int lineStart = position;
        int lineEnd   = position + 1;

        while( lineStart > 0 && source.charAt( lineStart ) != '\n')
            lineStart--;

        while( lineEnd < source.length() && source.charAt( lineEnd ) != '\n')
            lineEnd++;

        // Building the errLine (
        //     The one that will have the line
        //     number and the line from the source code.
        // )
        String indication = String.format("\t %d | ", line);
        String errLine   =  indication + source.substring(lineStart+1, lineEnd);


        // Calculating the number of spaces needed for ^ to be right below
        // the character where the error happened
        int errOffset = position - lineStart + indication.length() - 2;
        String spaces = String.format("\t%" + errOffset + "s", "");

        // Building the final String ...
        return String.format("%s: \n%s\n%s^-- Here.", msg, errLine, spaces);
    }
}
