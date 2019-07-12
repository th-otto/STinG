BOOLEAN	is_dcd		( WORD func );
void	high_dtr	( WORD func, MAPTAB *map );
void	low_dtr		( WORD func, MAPTAB *map );
BOOLEAN	SendBlock	( void *dev, BYTE *block, LONG len, BOOLEAN tst_dcd );
LONG	GetBlock	( void *dev, LONG bufflen, BYTE *buff );
void	SetMapM1	( MAPTAB **map );
void	SetMapMidi	( MAPTAB **map );
void	SetIorec	( IOREC *iorec, BYTE *blk, WORD len );
