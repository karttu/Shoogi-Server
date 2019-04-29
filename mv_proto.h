char *mv_getpiecename(struct GIS *gis,PIECE piece);
char *mv_getpieceshort(struct GIS *gis,UBYTE *dst,PIECE piece);
int mv_getkanjinum(struct GIS *gis,char *dst,int num);
struct GIS *mv_new_gis(int gametype,int board_size);
char *mv_getpiececode(struct GIS *gis,UBYTE *dst,int piece);
int mv_checkshortmoves(struct GIS *gis,COR *vec,int x,int y,int *p_capt_x,int *p_capt_y,PIECE thispiece,int movetype,int dir,int *moves);
int mv_checklongmoves(struct GIS *gis,COR *vec,int x,int y,int *p_capt_x,int *p_capt_y,PIECE thispiece,int movetype,int xx,int yy);
int mv_Shoogi_pawn_(struct GIS *gis,COR *vec,int x,int y,int *p_capt_x,int *p_capt_y,PIECE thispiece,int movetype,int dir);
int mv_Chess_pawn_(struct GIS *gis,COR *vec,int x,int y,int *p_capt_x,int *p_capt_y,PIECE thispiece,int movetype,int dir);
int mv_lance_(struct GIS *gis,COR *vec,int x,int y,int *p_capt_x,int *p_capt_y,PIECE thispiece,int movetype,int dir);
int mv_Shoogi_knight_(struct GIS *gis,COR *vec,int x,int y,int *p_capt_x,int *p_capt_y,PIECE thispiece,int movetype,int dir);
int mv_Chess_knight_(struct GIS *gis,COR *vec,int x,int y,int *p_capt_x,int *p_capt_y,PIECE thispiece,int movetype,int dir);
int mv_silver_(struct GIS *gis,COR *vec,int x,int y,int *p_capt_x,int *p_capt_y,PIECE thispiece,int movetype,int dir);
int mv_gold_(struct GIS *gis,COR *vec,int x,int y,int *p_capt_x,int *p_capt_y,PIECE thispiece,int movetype,int dir);
int mv_Shoogi_king_(struct GIS *gis,COR *vec,int x,int y,int *p_capt_x,int *p_capt_y,PIECE thispiece,int movetype,int dir);
int mv_Chess_king_(struct GIS *gis,COR *vec,int x,int y,int *p_capt_x,int *p_capt_y,PIECE thispiece,int movetype,int dir);
int mv_king_(struct GIS *gis,COR *vec,int x,int y,int *p_capt_x,int *p_capt_y,PIECE thispiece,int movetype,int dir);
int mv_bishop_(struct GIS *gis,COR *vec,int x,int y,int *p_capt_x,int *p_capt_y,PIECE thispiece,int movetype,int dir);
int mv_rook_(struct GIS *gis,COR *vec,int x,int y,int *p_capt_x,int *p_capt_y,PIECE thispiece,int movetype,int dir);
int mv_queen_(struct GIS *gis,COR *vec,int x,int y,int *p_capt_x,int *p_capt_y,PIECE thispiece,int movetype,int dir);
int mv_prom_bishop_(struct GIS *gis,COR *vec,int x,int y,int *p_capt_x,int *p_capt_y,PIECE thispiece,int movetype,int dir);
int mv_prom_rook_(struct GIS *gis,COR *vec,int x,int y,int *p_capt_x,int *p_capt_y,PIECE thispiece,int movetype,int dir);
int mv_bugmove_(struct GIS *gis,COR *vec,int x,int y,int *p_capt_x,int *p_capt_y,PIECE thispiece,int movetype,int dir);
int mv_checkedp(struct GIS *gis,COR *vec,int color);
int mv_king_checked_in_xy_p(struct GIS *gis,COR *vec,int color,int try_x,int try_y);
int mv_checkmatedp(struct GIS *gis,int color,char *escmsgbuf);
int mv_interpose_possible(struct GIS *gis,COR *vec,int x,int y,int x2,int y2,int color,char *escmsgbuf);
int mv_only_empty_squares_between(struct GIS *gis,int x,int y,int x2,int y2);
int mv_king_checked_in_first_n_squares(struct GIS *gis,COR *vec2,int x,int y,int x2,int y2,int color,int n);
int mv_castling_possible(struct GIS *gis,int color,char *errmsgbuf,int long_castling);
int mv_check_if_threatened(struct GIS *gis,COR *vec,int x,int y,int x2,int y2,int color,char *escmsgbuf,int movetype);
int mv_any_drop_possible(struct GIS *gis,PIECE *hand,int x,int y,int color,char *escmsgbuf);
int mv_find_threatening_pieces(struct GIS *gis,COR *vec,int x,int y,int color,int movetype);
int mv_valid_movep(struct GIS *gis,int src_x,int src_y,int dest_x,int dest_y,int *p_capt_x,int *p_capt_y,int piece,int movetype,int color);
int mv_find_available_squares(struct GIS *gis,COR *vec, int src_x, int src_y, int piece, int color);
int mv_handle_handicap(struct GIS *gis,char *movestr,int color,char *errmsgbuf);
int mv_valid_as_digit(struct GIS *gis,char c);
int mv_valid_as_letter(struct GIS *gis,char c);
int mv_valid_first(struct GIS *gis,char c);
int mv_valid_second(struct GIS *gis,char c);
int mv_parse_coordinate(struct GIS *gis,char **p_movestr,int *p_x,int *p_y);
int mv_execute_moves(struct GIS *gis,char *movelist, char *checkmsgbuf, char *errmsgbuf, char *escmsgbuf);
int mv_find_checkings(struct GIS *gis,int color, char *checkmsgbuf, char *escmsgbuf);
int mv_check_move(struct GIS *gis,char *movestr,int color, char *errmsgbuf);
int mv_illegal_drop(struct GIS *gis,int piece,int dest_x,int dest_y,int color,char *escmsgbuf);
int mv_unable_to_move(struct GIS *gis,int piece,int y,int color);
int mv_in_promzonep(struct GIS *gis,int y,int color);
char *mv_getcoord(struct GIS *gis,int x,int y);
char *mv_getcoord_b(struct GIS *gis,char *coordbuf,int x,int y);
int mv_sprintcoords_and(struct GIS *gis,char *dstbuf,COR *vec);
int mv_sprintcoords_or(struct GIS *gis,char *dstbuf,COR *vec);
int mv_inithand(struct GIS *gis,PIECE *hand);
int mv_add_piece_to_hand(struct GIS *gis,int piece,PIECE *hand);
int mv_remove_piece_from_hand(struct GIS *gis,int piece,PIECE *hand);
int mv_check_piece_in_hand(struct GIS *gis,int piece,PIECE *hand); /* Could be macro also. */
int mv_samepiece_in_file(struct GIS *gis,int x,int piece);
int mv_initmovefuns(struct GIS *gis);
int mv_initboard(struct GIS *gis);
int mv_Chess_init_board(struct GIS *gis);
int mv_Shoogi_init_board(struct GIS *gis);
int mv_countpieces(struct GIS *gis,int *b1count,int *w1count,int *b2count,int *w2count);
int mv_count_pieces_on_board(struct GIS *gis,int *b1count,int *w1count,int *b2count,int *w2count);
int mv_count_hand(struct GIS *gis,PIECE *hand,int *count2);
int mv_printboard(struct GIS *gis,int whichway,FILE *fp,ULI flags);
int mv_printhand(struct GIS *gis,PIECE *hand,int color,char *obuf);
int mv_print_hand(struct GIS *gis,PIECE *hand,int color,char *obuf);
char *mv_create_interline(struct GIS *gis,char *op);
int mv_print_file_labels(struct GIS *gis,int whichway,char *obuf);
