# PrintStringIntoString

This module prints a string into a formatted string.

**Detailed Description**

This module performs a sprintf operation with strings, whereever a '%s' in the format string is found the input string will be pasted. This module takes in a new string for each '%s' encountered in the format string. If not enough string are supplied an empty string is used.

**Note:** This module leaves any numeric formatting untouched. Hence statements like '%d' or '%f' remain in the string. To insert numbers, use the [PrintMatrixIntoString](PrintMatrixIntoString.md) module.
