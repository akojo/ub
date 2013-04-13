#ifndef STRBUF_H
#define STRBUF_H

struct strbuf;
typedef struct strbuf strbuf_t;

strbuf_t *strbuf_new();
void strbuf_delete(strbuf_t *buf);
void strbuf_reset(strbuf_t *buf);

void strbuf_append(strbuf_t *buf, char c);
char *strbuf_buffer(strbuf_t *buf);

#endif /* end of include guard: STRBUF_H */
