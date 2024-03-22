/**
 * https://www.gnu.org/software/libc/manual/html_node/iconv-Examples.html
 *
 * The example below features a solution for a common problem. Given that one
 * knows the internal encoding used by the system for wchar_t strings, one
 * often is in the position to read text from a file and store it in wide
 * character buffers. One can do this using mbsrtowcs(), but then we run into
 * the problems discussed above [elsewhere.
 *
 * This example shows the most important aspects of using the iconv functions.
 * It shows how successive calls to iconv can be used to convert large amounts
 * of text. The user does not have to care about stateful encodings as the
 * functions take care of everything.
 *
 * An interesting point is the case where iconv returns an error and errno is
 * set to EINVAL. This is not really an error in the transformation. It can
 * happen whenever the input character set contains byte sequences of more than
 * one byte for some character and texts are not processed in one piece. In
 * this case there is a chance that a multibyte sequence is cut. The caller can
 * then simply read the remainder of the takes and feed the offending bytes
 * together with new character from the input to iconv and continue the work.
 * The internal state kept in the descriptor is not unspecified after such an
 * event as is the case with the conversion functions from the ISO C standard.
 *
 * The example also shows the problem of using wide character strings with
 * iconv.  As explained in the description of the iconv function above, the
 * function always takes a pointer to a char array and the available space is
 * measured in bytes. In the example, the output buffer is a wide character
 * buffer; therefore, we use a local variable wrptr of type char *, which is
 * used in the iconv calls.
 *
 * This looks rather innocent but can lead to problems on platforms that have
 * tight restriction on alignment. Therefore the caller of iconv has to make
 * sure that the pointers passed are suitable for access of characters from the
 * appropriate character set. Since, in the above case, the input parameter to
 * the function is a wchar_t pointer, this is the case (unless the user
 * violates alignment when computing the parameter). But in other situations,
 * especially when writing generic functions where one does not know what type
 * of character set one uses and, therefore, treats text as a sequence of
 * bytes, it might become tricky.
 *
 */

int
file2wcs (int fd, const char * charset, wchar_t * outbuf, size_t avail)
{
    char inbuf[BUFSIZ];
    size_t insize = 0;
    char * wrptr = (char *) outbuf;
    int result = 0;
    iconv_t cd = iconv_open ("WCHAR_T", charset);
    if (cd == (iconv_t) -1)                     /* something went wrong     */
    {
        if (errno == EINVAL)
            error (0, 0, "conversion '%s' to wchar_t unavailable", charset);
        else
            perror ("iconv_open");

        *outbuf = L'\0';                        /* terminate output string  */
        return -1;
    }

    while (avail > 0)
    {
        size_t nread;
        size_t nconv;
        char * inptr = inbuf;

        /* Read more input.  */

        nread = read(fd, inbuf + insize, sizeof(inbuf) - insize);
        if (nread == 0)
        {
            /*
             * When we come here the file is completely read.  There could
             * still be unused characters in the inbuf.  Put them back. Then
             * write out the byte sequence to get into the initial state if
             * this is necessary.
             */

            if (lseek (fd, -insize, SEEK_CUR) == -1)
                result = -1;

            (void) iconv(cd, NULL, NULL, &wrptr, &avail);
            break;
        }
        insize += nread;

        /* Do the conversion.  */

        nconv = iconv(cd, &inptr, &insize, &wrptr, &avail);
        if (nconv == (size_t) -1)
        {
            /*
             * Not everything went right.  It might only be an unfinished byte
             * sequence at the end of the buffer.  Or it is a real problem.
             * This is harmless.  Simply move the unused bytes to the beginning
             * of the buffer so that they can be used in the next round.
             */

            if (errno == EINVAL)
            {
                memmove(inbuf, inptr, insize);
            }
            else
            {
                /*
                 * It is a real problem.  Maybe we ran out of space in the
                 * output buffer or we have invalid input.  In any case back
                 * the file pointer to the position of the last processed byte.
                 */

                lseek (fd, -insize, SEEK_CUR);
                result = -1;
                break;
            }
        }
    }

    if (avail >= sizeof(wchar_t))
        *((wchar_t *) wrptr) = L'\0';           /* terminate output string  */

    if (iconv_close(cd) != 0)
        perror ("iconv_close");

    return (wchar_t *) wrptr - outbuf;
}

/*
 * iconvfull.c
 *
 * vim: sw=4 ts=4 wm=4 et ft=c
 */
