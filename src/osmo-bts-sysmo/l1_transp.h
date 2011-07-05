#ifndef _FEMTOL1_TRANSPH_
#define _FEMTOL1_TRANSPH_

#include <osmocom/core/msgb.h>

/* functions a transport calls on arrival of primitive from BTS */
int l1if_handle_l1prim(struct femtol1_hdl *fl1h, struct msgb *msg);
int l1if_handle_sysprim(struct femtol1_hdl *fl1h, struct msgb *msg);
int l1if_handle_dbg(struct femtol1_hdl *fl1h, struct msgb *msg);

/* functions exported by a transport */
int l1if_transport_open(struct femtol1_hdl *fl1h);
int l1if_transport_close(struct femtol1_hdl *fl1h);

#endif /* _FEMTOL1_TRANSP_H */
