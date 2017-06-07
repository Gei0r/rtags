#ifndef WINDOWS_MKTEMP_H
#define WINDOWS_MKTEMP_H
#ifdef _WIN32

int windows_mkdtemp(char *tmpl);
int windows_mkstemp(char *tmpl);
int windows_mkostemp(char *tmpl, int flags);
int windows_mkstemps(char *tmpl, int suffixlen);
int windows_mkostemps(char *tmpl, int suffixlen, int flags);

#endif // _WIN32
#endif // WINDOWS_MKTEMP_H
