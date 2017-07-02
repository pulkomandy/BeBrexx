#ifdef __cplusplus
extern "C" {
#endif

void start_be_app(void);
void stop_be_app(void);

char *be_app_list(Lstr *, char, char);
long  be_app_exists(Lstr *);
long  redirect_be_app(Lstr *, Lstr *);

void  R_showlist(args *);

void be_set_var(char *, char *);
void be_set_cvar(char *, char *, char *);

#ifdef __cplusplus
}
#endif
