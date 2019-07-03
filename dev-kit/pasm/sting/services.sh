;----------------------------------------------------------------------------
; File name:	SERVICES.SH			Revision date:	1997.08.28
; Authors:	Ronald Andersson		Creation date:	1997.08.28
;(c)1997 by:	Ulf Ronald Andersson		All rights reserved
;Released as:	FREEWARE			(commercial sale forbidden)
;----------------------------------------------------------------------------
; Purpose:
;
;	Defines assemblytime constants for 'well known' Internet ports.
;	This file is based on RFC 1700/October 1994.
;
;----------------------------------------------------------------------------
;Required header declarations:
;
;	.include	"uran\struct.sh"
;	.include	"sting\_skelton.sh"
;
;copy the above to the header of your program and 'uncomment' the includes
;
;----------------------------------------------------------------------------
;
;Keyword		 Hex	 Dec.	Description
;-------		 --- 	 ----	------
RES_0x000_PORT	equ	$000	;   0	Reserved
TCPMUX_PORT	equ	$001	;   1	TCP Port Service Multiplexer
COMPRESSNET_2_PORT equ	$002	;   2	Management Utility
COMPRESSNET_3_PORT equ	$003	;   3	Compression Process
UNA_0x004_PORT	equ	$004	;   4	Unassigned
RJE_PORT	equ	$005	;   5	Remote Job Entry
UNA_0x006_PORT	equ	$006	;   6	Unassigned
ECHO_PORT	equ	$007	;   7	Echo
UNA_0x008_PORT	equ	$008	;   8	Unassigned
DISCARD_PORT	equ	$009	;   9	Discard
UNA_0x00A_PORT	equ	$00A	;  10	Unassigned
SYSTAT_PORT	equ	$00B	;  11	Active Users
UNA_0x00C_PORT	equ	$00C	;  12	Unassigned
DAYTIME_PORT	equ	$00D	;  13	Daytime
UNA_0x00E_PORT	equ	$00E	;  14	Unassigned
UNA_0x00F_PORT	equ	$00F	;  15	Unassigned [was netstat]
;
UNA_0x010_PORT	equ	$010	;  16	Unassigned
QOTD_PORT	equ	$011	;  17	Quote of the Day
MSP_PORT	equ	$012	;  18	Message Send Protocol
CHARGEN_PORT	equ	$013	;  19	Character Generator
FTP_DATA_PORT	equ	$014	;  20	File Transfer [Default Data]
FTP_PORT	equ	$015	;  21	File Transfer [Control]
UNA_0x016_PORT	equ	$016	;  22	Unassigned
TELNET_PORT	equ	$017	;  23	Telnet
PRIV_MAIL_PORT	equ	$018	;  24	any private mail system
SMTP_PORT	equ	$019	;  25	Simple Mail Transfer
UNA_0x01A_PORT	equ	$01A	;  26	Unassigned
NSW_FE_PORT	equ	$01B	;  27	NSW User System FE
UNA_0x01C_PORT	equ	$01C	;  28	Unassigned
MSG_ICP_PORT	equ	$01D	;  29	MSG ICP
UNA_0x01E_PORT	equ	$01E	;  30	Unassigned
MSG_AUTH_PORT	equ	$01F	;  31	MSG Authentication
;
UNA_0x020_PORT	equ	$020	;  32	Unassigned 
dsp              33/tcp    ;Display Support Protocol 
dsp              33/udp    ;Display Support Protocol 
;                          ;Ed Cain <cain@edn-unix.dca.mil> 
;                34/tcp    ;Unassigned 
;                34/udp    ;Unassigned 
                 35/tcp    ;any private printer server 
                 35/udp    ;any private printer server 
;                          ;Jon Postel <postel@isi.edu> 
;                36/tcp    ;Unassigned 
;                36/udp    ;Unassigned 
time             37/tcp    ;Time 
time             37/udp    ;Time 
;                          ;Jon Postel <postel@isi.edu> 
rap              38/tcp    ;Route Access Protocol 
rap              38/udp    ;Route Access Protocol 
;                          ;Robert Ullmann <ariel@world.std.com> 
rlp              39/tcp    ;Resource Location Protocol 
rlp              39/udp    ;Resource Location Protocol 
;                          ;Mike Accetta <MIKE.ACCETTA@CMU-CS-A.EDU> 
;                40/tcp    ;Unassigned 
;                40/udp    ;Unassigned 
graphics         41/tcp    ;Graphics 
graphics         41/udp    ;Graphics 
nameserver       42/tcp    ;Host Name Server 
nameserver       42/udp    ;Host Name Server 
nicname          43/tcp    ;Who Is 
nicname          43/udp    ;Who Is 
mpm-flags        44/tcp    ;MPM FLAGS Protocol 
mpm-flags        44/udp    ;MPM FLAGS Protocol 
mpm              45/tcp    ;Message Processing Module [recv] 
mpm              45/udp    ;Message Processing Module [recv] 
mpm-snd          46/tcp    ;MPM [default send] 
mpm-snd          46/udp    ;MPM [default send] 
;                          ;Jon Postel <postel@isi.edu> 
ni-ftp           47/tcp    ;NI FTP 
ni-ftp           47/udp    ;NI FTP 
;                          ;Steve Kille <S.Kille@isode.com> 
auditd           48/tcp    ;Digital Audit Daemon 
auditd           48/udp    ;Digital Audit Daemon 
;                          ;Larry Scott <scott@zk3.dec.com> 
login            49/tcp    ;Login Host Protocol 
login            49/udp    ;Login Host Protocol 
;                          ;Pieter Ditmars <pditmars@BBN.COM> 
re-mail-ck       50/tcp    ;Remote Mail Checking Protocol 
re-mail-ck       50/udp    ;Remote Mail Checking Protocol 
;                          ;Steve Dorner <s-dorner@UIUC.EDU> 
la-maint         51/tcp    ;IMP Logical Address Maintenance 
la-maint         51/udp    ;IMP Logical Address Maintenance 
;                          ;Andy Malis <malis_a@timeplex.com> 
xns-time         52/tcp    ;XNS Time Protocol 
xns-time         52/udp    ;XNS Time Protocol 
;                          ;Susie Armstrong <Armstrong.wbst128@XEROX> 
domain           53/tcp    dns DNS ;Domain Name Server 
domain           53/udp    dns DNS ;Domain Name Server 
;                          ;Paul Mockapetris <PVM@ISI.EDU> 
xns-ch           54/tcp    ;XNS Clearinghouse 
xns-ch           54/udp    ;XNS Clearinghouse 
;                          ;Susie Armstrong <Armstrong.wbst128@XEROX> 
isi-gl           55/tcp    ;ISI Graphics Language 
isi-gl           55/udp    ;ISI Graphics Language 
xns-auth         56/tcp    ;XNS Authentication 
xns-auth         56/udp    ;XNS Authentication 
;                          ;Susie Armstrong <Armstrong.wbst128@XEROX> 
                 57/tcp    ;any private terminal access 
                 57/udp    ;any private terminal access 
;                          ;Jon Postel <postel@isi.edu> 
xns-mail         58/tcp    ;XNS Mail 
xns-mail         58/udp    ;XNS Mail 
;                          ;Susie Armstrong <Armstrong.wbst128@XEROX> 
                 59/tcp    ;any private file service 
                 59/udp    ;any private file service 
;                          ;Jon Postel <postel@isi.edu> 
                 60/tcp    ;Unassigned 
                 60/udp    ;Unassigned 
ni-mail          61/tcp    ;NI MAIL 
ni-mail          61/udp    ;NI MAIL 
;                          ;Steve Kille <S.Kille@isode.com> 
acas             62/tcp    ;ACA Services 
acas             62/udp    ;ACA Services 
;                          ;E. Wald <ewald@via.enet.dec.com> 
;                63/tcp    ;Unassigned 
;                63/udp    ;Unassigned 
covia            64/tcp    ;Communications Integrator (CI) 
covia            64/udp    ;Communications Integrator (CI) 
;                          ;"Tundra" Tim Daneliuk 
;                          ;<tundraix!tundra@clout.chi.il.us> 
tacacs-ds        65/tcp    ;TACACS-Database Service 
tacacs-ds        65/udp    ;TACACS-Database Service 
;                          ;Kathy Huber <khuber@bbn.com> 
sql*net          66/tcp    ;Oracle SQL*NET 
sql*net          66/udp    ;Oracle SQL*NET 
;                          ;Jack Haverty <jhaverty@ORACLE.COM> 
bootps           67/tcp    ;Bootstrap Protocol Server 
bootps           67/udp    ;Bootstrap Protocol Server 
bootpc           68/tcp    ;Bootstrap Protocol Client 
bootpc           68/udp    ;Bootstrap Protocol Client 
;                          ;Bill Croft <Croft@SUMEX-AIM.STANFORD.EDU> 
tftp             69/tcp    ;Trivial File Transfer 
tftp             69/udp    ;Trivial File Transfer 
;                          ;David Clark <ddc@LCS.MIT.EDU> 
gopher           70/tcp    ;Gopher 
gopher           70/udp    ;Gopher 
;                          ;Mark McCahill <mpm@boombox.micro.umn.edu> 
netrjs-1         71/tcp    ;Remote Job Service 
netrjs-1         71/udp    ;Remote Job Service 
netrjs-2         72/tcp    ;Remote Job Service 
netrjs-2         72/udp    ;Remote Job Service 
netrjs-3         73/tcp    ;Remote Job Service 
netrjs-3         73/udp    ;Remote Job Service 
netrjs-4         74/tcp    ;Remote Job Service 
netrjs-4         74/udp    ;Remote Job Service 
;                          ;Bob Braden <Braden@ISI.EDU> 
                 75/tcp    ;any private dial out service 
                 75/udp    ;any private dial out service 
;                          ;Jon Postel <postel@isi.edu> 
deos             76/tcp    ;Distributed External Object Store 
deos             76/udp    ;Distributed External Object Store 
;                          ;Robert Ullmann <ariel@world.std.com> 
                 77/tcp    ;any private RJE service 
                 77/udp    ;any private RJE service 
;                          ;Jon Postel <postel@isi.edu> 
vettcp           78/tcp    ;vettcp 
vettcp           78/udp    ;vettcp 
;                          ;Christopher Leong <leong@kolmod.mlo.dec.com> 
finger           79/tcp    ;Finger 
finger           79/udp    ;Finger 
;                          ;David Zimmerman <dpz@RUTGERS.EDU> 
www-http         80/tcp    http ;World Wide Web HTTP 
www-http         80/udp    http ;World Wide Web HTTP 
;                          ;Tim Berners-Lee <timbl@nxoc01.cern.ch> 
hosts2-ns        81/tcp    ;HOSTS2 Name Server 
hosts2-ns        81/udp    ;HOSTS2 Name Server 
;                          ;Earl Killian <EAK@MORDOR.S1.GOV> 
xfer             82/tcp    ;XFER Utility 
xfer             82/udp    ;XFER Utility 
;                          ;Thomas M. Smith <tmsmith@esc.syr.ge.com> 
mit-ml-dev       83/tcp    ;MIT ML Device 
mit-ml-dev       83/udp    ;MIT ML Device 
;                          ;David Reed <--none---> 
ctf              84/tcp    ;Common Trace Facility 
ctf              84/udp    ;Common Trace Facility 
;                          ;Hugh Thomas <thomas@oils.enet.dec.com> 
mit-ml-dev       85/tcp    ;MIT ML Device 
mit-ml-dev       85/udp    ;MIT ML Device 
;                          ;David Reed <--none---> 
mfcobol          86/tcp    ;Micro Focus Cobol 
mfcobol          86/udp    ;Micro Focus Cobol 
;                          ;Simon Edwards <--none---> 
                 87/tcp    ;any private terminal link 
                 87/udp    ;any private terminal link 
;                          ;Jon Postel <postel@isi.edu> 
kerberos         88/tcp    ;Kerberos 
kerberos         88/udp    ;Kerberos 
;                          ;B. Clifford Neuman <bcn@isi.edu> 
su-mit-tg        89/tcp    ;SU/MIT Telnet Gateway 
su-mit-tg        89/udp    ;SU/MIT Telnet Gateway 
;                          ;Mark Crispin <MRC@PANDA.COM> 
dnsix            90/tcp    ;DNSIX Securit Attribute Token Map 
dnsix            90/udp    ;DNSIX Securit Attribute Token Map 
;                          ;Charles Watt <watt@sware.com> 
mit-dov          91/tcp    ;MIT Dover Spooler 
mit-dov          91/udp    ;MIT Dover Spooler 
;                          ;Eliot Moss <EBM@XX.LCS.MIT.EDU> 
npp              92/tcp    ;Network Printing Protocol 
npp              92/udp    ;Network Printing Protocol 
;                          ;Louis Mamakos <louie@sayshell.umd.edu> 
dcp              93/tcp    ;Device Control Protocol 
dcp              93/udp    ;Device Control Protocol 
;                          ;Daniel Tappan <Tappan@BBN.COM> 
objcall          94/tcp    ;Tivoli Object Dispatcher 
objcall          94/udp    ;Tivoli Object Dispatcher 
;                          ;Tom Bereiter <--none---> 
supdup           95/tcp    ;SUPDUP 
supdup           95/udp    ;SUPDUP 
;                          ;Mark Crispin <MRC@PANDA.COM> 
dixie            96/tcp    ;DIXIE Protocol Specification 
dixie            96/udp    ;DIXIE Protocol Specification 
;                Tim Howes ;<Tim.Howes@terminator.cc.umich.edu> 
swift-rvf        97/tcp    ;Swift Remote Vitural File Protocol 
swift-rvf        97/udp    ;Swift Remote Vitural File Protocol 
;                          ;Maurice R. Turcotte 
;                <mailrus!uflorida!rm1!dnmrt%rmatl@uunet.UU.NET> 
tacnews          98/tcp    ;TAC News 
tacnews          98/udp    ;TAC News 
;                          ;Jon Postel <postel@isi.edu> 
metagram         99/tcp    ;Metagram Relay 
metagram         99/udp    ;Metagram Relay 
;                          ;Geoff Goodfellow <Geoff@FERNWOOD.MPK.CA.U> 
newacct         100/tcp    ;[unauthorized use] 
hostname        101/tcp    ;NIC Host Name Server 
hostname        101/udp    ;NIC Host Name Server 
;                          ;Jon Postel <postel@isi.edu> 
iso-tsap        102/tcp    ;ISO-TSAP 
iso-tsap        102/udp    ;ISO-TSAP 
;                          ;Marshall Rose <mrose@dbc.mtview.ca.us> 
gppitnp         103/tcp    ;Genesis Point-to-Point Trans Net 
gppitnp         103/udp    ;Genesis Point-to-Point Trans Net 
acr-nema        104/tcp    ;ACR-NEMA Digital Imag. & Comm. 300 
acr-nema        104/udp    ;ACR-NEMA Digital Imag. & Comm. 300 
;                          ;Patrick McNamee <--none---> 
csnet-ns        105/tcp    ;Mailbox Name Nameserver 
csnet-ns        105/udp    ;Mailbox Name Nameserver 
;                          ;Marvin Solomon <solomon@CS.WISC.EDU> 
3com-tsmux      106/tcp    ;3COM-TSMUX 
3com-tsmux      106/udp    ;3COM-TSMUX 
;                          ;Jeremy Siegel <jzs@NSD.3Com.COM> 
rtelnet         107/tcp    ;Remote Telnet Service 
rtelnet         107/udp    ;Remote Telnet Service 
;                          ;Jon Postel <postel@isi.edu> 
snagas          108/tcp    ;SNA Gateway Access Server 
snagas          108/udp    ;SNA Gateway Access Server 
;                          ;Kevin Murphy <murphy@sevens.lkg.dec.com> 
pop2            109/tcp    ;Post Office Protocol - Version 2 
pop2            109/udp    ;Post Office Protocol - Version 2 
;                          ;Joyce K. Reynolds <jkrey@isi.edu> 
pop3            110/tcp    ;Post Office Protocol - Version 3 
pop3            110/udp    ;Post Office Protocol - Version 3 
;                          ;Marshall Rose <mrose@dbc.mtview.ca.us> 
sunrpc          111/tcp    ;SUN Remote Procedure Call 
sunrpc          111/udp    ;SUN Remote Procedure Call 
;                          ;Chuck McManis <cmcmanis@sun.com> 
mcidas          112/tcp    ;McIDAS Data Transmission Protocol 
mcidas          112/udp    ;McIDAS Data Transmission Protocol 
;                          ;Glenn Davis <davis@unidata.ucar.edu> 
auth            113/tcp    ;Authentication Service 
auth            113/udp    ;Authentication Service 
;                          ;Mike St. Johns <stjohns@arpa.mil> 
audionews       114/tcp    ;Audio News Multicast 
audionews       114/udp    ;Audio News Multicast 
;                          ;Martin Forssen <maf@dtek.chalmers.se> 
sftp            115/tcp    ;Simple File Transfer Protocol 
sftp            115/udp    ;Simple File Transfer Protocol 
;                          ;Mark Lottor <MKL@nisc.sri.com> 
ansanotify      116/tcp    ;ANSA REX Notify 
ansanotify      116/udp    ;ANSA REX Notify 
;                          ;Nicola J. Howarth <njh@ansa.co.uk> 
uucp-path       117/tcp    ;UUCP Path Service 
uucp-path       117/udp    ;UUCP Path Service 
sqlserv         118/tcp    ;SQL Services 
sqlserv         118/udp    ;SQL Services 
;                          ;Larry Barnes <barnes@broke.enet.dec.com> 
nntp            119/tcp    ;Network News Transfer Protocol 
nntp            119/udp    ;Network News Transfer Protocol 
;                          ;Phil Lapsley <phil@UCBARPA.BERKELEY.EDU> 
cfdptkt         120/tcp    ;CFDPTKT 
cfdptkt         120/udp    ;CFDPTKT 
;                          ;John Ioannidis <ji@close.cs.columbia.ed> 
erpc            121/tcp    ;Encore Expedited Remote Pro.Call 
erpc            121/udp    ;Encore Expedited Remote Pro.Call 
;                          ;Jack O'Neil <---none---> 
smakynet        122/tcp    ;SMAKYNET 
smakynet        122/udp    ;SMAKYNET 
;                          ;Mike O'Dowd <odowd@ltisun8.epfl.ch> 
ntp             123/tcp    ;Network Time Protocol 
ntp             123/udp    ;Network Time Protocol 
;                          ;Dave Mills <Mills@HUEY.UDEL.EDU> 
ansatrader      124/tcp    ;ANSA REX Trader 
ansatrader      124/udp    ;ANSA REX Trader 
;                          ;Nicola J. Howarth <njh@ansa.co.uk> 
locus-map       125/tcp    ;Locus PC-Interface Net Map Ser 
locus-map       125/udp    ;Locus PC-Interface Net Map Ser 
;                          ;Eric Peterson <lcc.eric@SEAS.UCLA.EDU> 
unitary         126/tcp    ;Unisys Unitary Login 
unitary         126/udp    ;Unisys Unitary Login 
;                          ;<feil@kronos.nisd.cam.unisys.com> 
locus-con       127/tcp    ;Locus PC-Interface Conn Server 
locus-con       127/udp    ;Locus PC-Interface Conn Server 
;                          ;Eric Peterson <lcc.eric@SEAS.UCLA.EDU> 
gss-xlicen      128/tcp    ;GSS X License Verification 
gss-xlicen      128/udp    ;GSS X License Verification 
;                          ;John Light <johnl@gssc.gss.com> 
pwdgen          129/tcp    ;Password Generator Protocol 
pwdgen          129/udp    ;Password Generator Protocol 
;               Frank J. Wacho <WANCHO@WSMR-SIMTEL20.ARMY.MIL> 
cisco-fna       130/tcp    ;cisco FNATIVE 
cisco-fna       130/udp    ;cisco FNATIVE 
cisco-tna       131/tcp    ;cisco TNATIVE 
cisco-tna       131/udp    ;cisco TNATIVE 
cisco-sys       132/tcp    ;cisco SYSMAINT 
cisco-sys       132/udp    ;cisco SYSMAINT 
statsrv         133/tcp    ;Statistics Service 
statsrv         133/udp    ;Statistics Service 
;                          ;Dave Mills <Mills@HUEY.UDEL.EDU> 
ingres-net      134/tcp    ;INGRES-NET Service 
ingres-net      134/udp    ;INGRES-NET Service 
;                          ;Mike Berrow <---none---> 
loc-srv         135/tcp    ;Location Service 
loc-srv         135/udp    ;Location Service 
;                          ;Joe Pato <apollo!pato@EDDIE.MIT.EDU> 
profile         136/tcp    ;PROFILE Naming System 
profile         136/udp    ;PROFILE Naming System 
;                          ;Larry Peterson <llp@ARIZONA.EDU> 
netbios-ns      137/tcp    ;NETBIOS Name Service 
netbios-ns      137/udp    ;NETBIOS Name Service 
netbios-dgm     138/tcp    ;NETBIOS Datagram Service 
netbios-dgm     138/udp    ;NETBIOS Datagram Service 
netbios-ssn     139/tcp    ;NETBIOS Session Service 
netbios-ssn     139/udp    ;NETBIOS Session Service 
;                          ;Jon Postel <postel@isi.edu> 
emfis-data      140/tcp    ;EMFIS Data Service 
emfis-data      140/udp    ;EMFIS Data Service 
emfis-cntl      141/tcp    ;EMFIS Control Service 
emfis-cntl      141/udp    ;EMFIS Control Service 
;                          ;Gerd Beling <GBELING@ISI.EDU> 
bl-idm          142/tcp    ;Britton-Lee IDM 
bl-idm          142/udp    ;Britton-Lee IDM 
;                          ;Susie Snitzer <---none---> 
imap2           143/tcp    ;Interim Mail Access Protocol v2 
imap2           143/udp    ;Interim Mail Access Protocol v2 
;                          ;Mark Crispin <MRC@PANDA.COM> 
news            144/tcp    ;NewS 
news            144/udp    ;NewS 
;                          ;James Gosling <JAG@SUN.COM> 
uaac            145/tcp    ;UAAC Protocol 
uaac            145/udp    ;UAAC Protocol 
;               David A. Gomberg <gomberg@GATEWAY.MITRE.ORG> 
iso-tp0         146/tcp    ;ISO-IP0 
iso-tp0         146/udp    ;ISO-IP0 
iso-ip          147/tcp    ;ISO-IP 
iso-ip          147/udp    ;ISO-IP 
;                          ;Marshall Rose <mrose@dbc.mtview.ca.us> 
cronus          148/tcp    ;CRONUS-SUPPORT 
cronus          148/udp    ;CRONUS-SUPPORT 
;                          ;Jeffrey Buffun <jbuffum@APOLLO.COM> 
aed-512         149/tcp    ;AED 512 Emulation Service 
aed-512         149/udp    ;AED 512 Emulation Service 
;               Albert G. Broscius <broscius@DSL.CIS.UPENN.EDU> 
sql-net         150/tcp    ;SQL-NET 
sql-net         150/udp    ;SQL-NET 
;                          ;Martin Picard <<---none---> 
hems            151/tcp    ;HEMS 
hems            151/udp    ;HEMS 
;                          ;Christopher Tengi <tengi@Princeton.EDU> 
bftp            152/tcp    ;Background File Transfer Program 
bftp            152/udp    ;Background File Transfer Program 
;                          ;Annette DeSchon <DESCHON@ISI.EDU> 
sgmp            153/tcp    ;SGMP 
sgmp            153/udp    ;SGMP 
;                          ;Marty Schoffstahl <schoff@NISC.NYSER.NET> 
netsc-prod      154/tcp    ;NETSC 
netsc-prod      154/udp    ;NETSC 
netsc-dev       155/tcp    ;NETSC 
netsc-dev       155/udp    ;NETSC 
;                          ;Sergio Heker <heker@JVNCC.CSC.ORG> 
sqlsrv          156/tcp    ;SQL Service 
sqlsrv          156/udp    ;SQL Service 
;                          ;Craig Rogers <Rogers@ISI.EDU> 
knet-cmp        157/tcp    ;KNET/VM Command/Message Protocol 
knet-cmp        157/udp    ;KNET/VM Command/Message Protocol 
;                          ;Gary S. Malkin <GMALKIN@XYLOGICS.COM> 
pcmail-srv      158/tcp    ;PCMail Server 
pcmail-srv      158/udp    ;PCMail Server 
;                          ;Mark L. Lambert <markl@PTT.LCS.MIT.EDU> 
nss-routing     159/tcp    ;NSS-Routing 
nss-routing     159/udp    ;NSS-Routing 
;                          ;Yakov Rekhter <Yakov@IBM.COM> 
sgmp-traps      160/tcp    ;SGMP-TRAPS 
sgmp-traps      160/udp    ;SGMP-TRAPS 
;                          ;Marty Schoffstahl <schoff@NISC.NYSER.NET> 
snmp            161/tcp    ;SNMP 
snmp            161/udp    ;SNMP 
snmptrap        162/tcp    ;SNMPTRAP 
snmptrap        162/udp    ;SNMPTRAP 
;                          ;Marshall Rose <mrose@dbc.mtview.ca.us> 
cmip-man        163/tcp    ;CMIP/TCP Manager 
cmip-man        163/udp    ;CMIP/TCP Manager 
cmip-agent      164/tcp    ;CMIP/TCP Agent 
smip-agent      164/udp    ;CMIP/TCP Agent 
;                          ;Amatzia Ben-Artzi <---none---> 
xns-courier     165/tcp    ;Xerox 
xns-courier     165/udp    ;Xerox 
;                          ;Susie Armstrong <Armstrong.wbst128@XEROX.COM> 
s-net           166/tcp    ;Sirius Systems 
s-net           166/udp    ;Sirius Systems 
;                          ;Brian Lloyd <---none---> 
namp            167/tcp    ;NAMP 
namp            167/udp    ;NAMP 
;                          ;Marty Schoffstahl <schoff@NISC.NYSER.NET> 
rsvd            168/tcp    ;RSVD 
rsvd            168/udp    ;RSVD 
;                          ;Neil Todd <mcvax!ist.co.uk!neil@UUNET.UU.NET> 
send            169/tcp    ;SEND 
send            169/udp    ;SEND 
;               William D. ;Wisner <wisner@HAYES.FAI.ALASKA.EDU> 
print-srv       170/tcp    ;Network PostScript 
print-srv       170/udp    ;Network PostScript 
;                          ;Brian Reid <reid@DECWRL.DEC.COM> 
multiplex       171/tcp    ;Network Innovations Multiplex 
multiplex       171/udp    ;Network Innovations Multiplex 
cl/1            172/tcp    ;Network Innovations CL/1 
cl/1            172/udp    ;Network Innovations CL/1 
;                          ;Kevin DeVault <<---none---> 
xyplex-mux      173/tcp    ;Xyplex 
xyplex-mux      173/udp    ;Xyplex 
;                          ;Bob Stewart <STEWART@XYPLEX.COM> 
mailq           174/tcp    ;MAILQ 
mailq           174/udp    ;MAILQ 
;                          ;Rayan Zachariassen <rayan@AI.TORONTO.EDU> 
vmnet           175/tcp    ;VMNET 
vmnet           175/udp    ;VMNET 
;                          ;Christopher Tengi <tengi@Princeton.EDU> 
genrad-mux      176/tcp    ;GENRAD-MUX 
genrad-mux      176/udp    ;GENRAD-MUX 
;                          ;Ron Thornton <thornton@qm7501.genrad.com> 
xdmcp           177/tcp    ;X Display Manager Control Protocol 
xdmcp           177/udp    ;X Display Manager Control Protocol 
;                          ;Robert W. Scheifler <RWS@XX.LCS.MIT.EDU> 
nextstep        178/tcp    ;NextStep Window Server 
NextStep        178/udp    ;NextStep Window Server 
;                          ;Leo Hourvitz <leo@NEXT.COM> 
bgp             179/tcp    ;Border Gateway Protocol 
bgp             179/udp    ;Border Gateway Protocol 
;                          ;Kirk Lougheed <LOUGHEED@MATHOM.CISCO.COM> 
ris             180/tcp    ;Intergraph 
ris             180/udp    ;Intergraph 
;                          ;Dave Buehmann <ingr!daveb@UUNET.UU.NET> 
unify           181/tcp    ;Unify 
unify           181/udp    ;Unify 
;                          ;Vinod Singh <--none---> 
audit           182/tcp    ;Unisys Audit SITP 
audit           182/udp    ;Unisys Audit SITP 
;                          ;Gil Greenbaum <gcole@nisd.cam.unisys.com> 
ocbinder        183/tcp    ;OCBinder 
ocbinder        183/udp    ;OCBinder 
ocserver        184/tcp    ;OCServer 
ocserver        184/udp    ;OCServer 
;                          ;Jerrilynn Okamura <--none---> 
remote-kis      185/tcp    ;Remote-KIS 
remote-kis      185/udp    ;Remote-KIS 
kis             186/tcp    ;KIS Protocol 
kis             186/udp    ;KIS Protocol 
;                          ;Ralph Droms <rdroms@NRI.RESTON.VA.US> 
aci             187/tcp    ;Application Communication Interface 
aci             187/udp    ;Application Communication Interface 
;                          ;Rick Carlos <rick.ticipa.csc.ti.com> 
mumps           188/tcp    ;Plus Five's MUMPS 
mumps           188/udp    ;Plus Five's MUMPS 
;                          ;Hokey Stenn <hokey@PLUS5.COM> 
qft             189/tcp    ;Queued File Transport 
qft             189/udp    ;Queued File Transport 
;                          ;Wayne Schroeder <schroeder@SDS.SDSC.EDU> 
gacp            190/tcp    ;Gateway Access Control Protocol 
cacp            190/udp    ;Gateway Access Control Protocol 
;                          ;C. Philip Wood <cpw@LANL.GOV> 
prospero        191/tcp    ;Prospero Directory Service 
prospero        191/udp    ;Prospero Directory Service 
;                          ;B. Clifford Neuman <bcn@isi.edu> 
osu-nms         192/tcp    ;OSU Network Monitoring System 
osu-nms         192/udp    ;OSU Network Monitoring System 
;               Doug Karl <KARL-D@OSU-20.IRCC.OHIO-STATE.EDU> 
srmp            193/tcp    ;Spider Remote Monitoring Protocol 
srmp            193/udp    ;Spider Remote Monitoring Protocol 
;                          ;Ted J. Socolofsky <Teds@SPIDER.CO.UK> 
irc             194/tcp    ;Internet Relay Chat Protocol 
irc             194/udp    ;Internet Relay Chat Protocol 
;                          ;Jarkko Oikarinen <jto@TOLSUN.OULU.FI> 
dn6-nlm-aud     195/tcp    ;DNSIX Network Level Module Audit 
dn6-nlm-aud     195/udp    ;DNSIX Network Level Module Audit 
dn6-smm-red     196/tcp    ;DNSIX Session Mgt Module Audit Redir 
dn6-smm-red     196/udp    ;DNSIX Session Mgt Module Audit Redir 
;                          ;Lawrence Lebahn <DIA3@PAXRV-NES.NAVY.MIL> 
dls             197/tcp    ;Directory Location Service 
dls             197/udp    ;Directory Location Service 
dls-mon         198/tcp    ;Directory Location Service Monitor 
dls-mon         198/udp    ;Directory Location Service Monitor 
;                          ;Scott Bellew <smb@cs.purdue.edu> 
smux            199/tcp    ;SMUX 
smux            199/udp    ;SMUX 
;                          ;Marshall Rose <mrose@dbc.mtview.ca.us> 
src             200/tcp    ;IBM System Resource Controller 
src             200/udp    ;IBM System Resource Controller 
;                          ;Gerald McBrearty <---none---> 
at-rtmp         201/tcp    ;AppleTalk Routing Maintenance 
at-rtmp         201/udp    ;AppleTalk Routing Maintenance 
at-nbp          202/tcp    ;AppleTalk Name Binding 
at-nbp          202/udp    ;AppleTalk Name Binding 
at-3            203/tcp    ;AppleTalk Unused 
at-3            203/udp    ;AppleTalk Unused 
at-echo         204/tcp    ;AppleTalk Echo 
at-echo         204/udp    ;AppleTalk Echo 
at-5            205/tcp    ;AppleTalk Unused 
at-5            205/udp    ;AppleTalk Unused 
at-zis          206/tcp    ;AppleTalk Zone Information 
at-zis          206/udp    ;AppleTalk Zone Information 
at-7            207/tcp    ;AppleTalk Unused 
at-7            207/udp    ;AppleTalk Unused 
at-8            208/tcp    ;AppleTalk Unused 
at-8            208/udp    ;AppleTalk Unused 
;                          ;Rob Chandhok <chandhok@gnome.cs.cmu.edu> 
tam             209/tcp    ;Trivial Authenticated Mail Protocol 
tam             209/udp    ;Trivial Authenticated Mail Protocol 
;                          ;Dan Bernstein <brnstnd@stealth.acf.nyu.edu> 
z39.50          210/tcp    ;ANSI Z39.50 
z39.50          210/udp    ;ANSI Z39.50 
;                          ;Mark Needleman 
;                         <mhnur%uccmvsa.bitnet@cornell.cit.cornell.edu> 
914c/g          211/tcp    ;Texas Instruments 914C/G Terminal 
914c/g          211/udp    ;Texas Instruments 914C/G Terminal 
;                          ;Bill Harrell <---none---> 
anet            212/tcp    ;ATEXSSTR 
anet            212/udp    ;ATEXSSTR 
;                          ;Jim Taylor <taylor@heart.epps.kodak.com> 
ipx             213/tcp    ;IPX 
ipx             213/udp    ;IPX 
;                          ;Don Provan <donp@xlnvax.novell.com> 
vmpwscs         214/tcp    ;VM PWSCS 
vmpwscs         214/udp    ;VM PWSCS 
;                          ;Dan Shia <dset!shia@uunet.UU.NET> 
softpc          215/tcp    ;Insignia Solutions 
softpc          215/udp    ;Insignia Solutions 
;                          ;Martyn Thomas <---none---> 
atls            216/tcp    ;Access Technology License Server 
atls            216/udp    ;Access Technology License Server 
;                          ;Larry DeLuca <henrik@EDDIE.MIT.EDU> 
dbase           217/tcp    ;dBASE Unix 
dbase           217/udp    ;dBASE Unix 
;                          ;Don Gibson 
;            <sequent!aero!twinsun!ashtate.A-T.COM!dong@uunet.UU.NET> 
mpp             218/tcp    ;Netix Message Posting Protocol 
mpp             218/udp    ;Netix Message Posting Protocol 
;                          ;Shannon Yeh <yeh@netix.com> 
uarps           219/tcp    ;Unisys ARPs 
uarps           219/udp    ;Unisys ARPs 
;                          ;Ashok Marwaha <---none---> 
imap3           220/tcp    ;Interactive Mail Access Protocol v3 
imap3           220/udp    ;Interactive Mail Access Protocol v3 
;                          ;James Rice <RICE@SUMEX-AIM.STANFORD.EDU> 
fln-spx         221/tcp    ;Berkeley rlogind with SPX auth 
fln-spx         221/udp    ;Berkeley rlogind with SPX auth 
rsh-spx         222/tcp    ;Berkeley rshd with SPX auth 
rsh-spx         222/udp    ;Berkeley rshd with SPX auth 
cdc             223/tcp    ;Certificate Distribution Center 
cdc             223/udp    ;Certificate Distribution Center 
;               Kannan Alagappan <kannan@sejour.enet.dec.com> 
;               224-241    ;Reserved 
;                          ;Jon Postel <postel@isi.edu> 
;               242/tcp    ;Unassigned 
;               242/udp    ;Unassigned 
sur-meas        243/tcp    ;Survey Measurement 
sur-meas        243/udp    ;Survey Measurement 
;                          ;Dave Clark <ddc@LCS.MIT.EDU> 
;               244/tcp    ;Unassigned 
;               244/udp    ;Unassigned 
link            245/tcp    ;LINK 
link            245/udp    ;LINK 
dsp3270         246/tcp    ;Display Systems Protocol 
dsp3270         246/udp    ;Display Systems Protocol 
;                          ;Weldon J. Showalter <Gamma@MINTAKA.DCA.MIL> 
;               247-255    ;Reserved 
;                          ;Jon Postel <postel@isi.edu> 
;               256-343    ;Unassigned 
pdap            344/tcp    ;Prospero Data Access Protocol 
pdap            344/udp    ;Prospero Data Access Protocol 
;                          ;B. Clifford Neuman <bcn@isi.edu> 
pawserv         345/tcp    ;Perf Analysis Workbench 
pawserv         345/udp    ;Perf Analysis Workbench 
zserv           346/tcp    ;Zebra server 
zserv           346/udp    ;Zebra server 
fatserv         347/tcp    ;Fatmen Server 
fatserv         347/udp    ;Fatmen Server 
csi-sgwp        348/tcp    ;Cabletron Management Protocol 
csi-sgwp        348/udp    ;Cabletron Management Protocol 
;               349-370    ;Unassigned 
clearcase       371/tcp    ;Clearcase 
clearcase       371/udp    ;Clearcase 
;                          ;Dave LeBlang <leglang@atria.com> 
ulistserv       372/tcp    ;Unix Listserv 
ulistserv       372/udp    ;Unix Listserv 
;                          ;Anastasios Kotsikonas <tasos@cs.bu.edu> 
legent-1        373/tcp    ;Legent Corporation 
legent-1        373/udp    ;Legent Corporation 
legent-2        374/tcp    ;Legent Corporation 
legent-2        374/udp    ;Legent Corporation 
;                          ;Keith Boyce <---none---> 
hassle          375/tcp    ;Hassle 
hassle          375/udp    ;Hassle 
;                          ;Reinhard Doelz <doelz@comp.bioz.unibas.ch> 
nip             376/tcp    ;Amiga Envoy Network Inquiry Proto 
nip             376/udp    ;Amiga Envoy Network Inquiry Proto 
;                          ;Kenneth Dyke <kcd@cbmvax.cbm.commodore.com> 
tnETOS          377/tcp    ;NEC Corporation 
tnETOS          377/udp    ;NEC Corporation 
dsETOS          378/tcp    ;NEC Corporation 
dsETOS          378/udp    ;NEC Corporation 
;                          ;Tomoo Fujita <tf@arc.bs1.fc.nec.co.jp> 
is99c           379/tcp    ;TIA/EIA/IS-99 modem client 
is99c           379/udp    ;TIA/EIA/IS-99 modem client 
is99s           380/tcp    ;TIA/EIA/IS-99 modem server 
is99s           380/udp    ;TIA/EIA/IS-99 modem server 
;                          ;Frank Quick <fquick@qualcomm.com> 
hp-collector    381/tcp    ;hp performance data collector 
hp-collector    381/udp    ;hp performance data collector 
hp-managed-node 382/tcp    ;hp performance data managed node 
hp-managed-node 382/udp    ;hp performance data managed node 
hp-alarm-mgr    383/tcp    ;hp performance data alarm manager 
hp-alarm-mgr    383/udp    ;hp performance data alarm manager 
;                          ;Frank Blakely <frankb@hpptc16.rose.hp.com> 
arns            384/tcp    ;A Remote Network Server System 
arns            384/udp    ;A Remote Network Server System 
;                          ;David Hornsby <djh@munnari.OZ.AU> 
ibm-app         385/tcp    ;IBM Application 
ibm-app         385/tcp    ;IBM Application 
;                          ;Lisa Tomita <---none---> 
asa             386/tcp    ;ASA Message Router Object Def. 
asa             386/udp    ;ASA Message Router Object Def. 
;                          ;Steve Laitinen <laitinen@brutus.aa.ab.com> 
aurp            387/tcp    ;Appletalk Update-Based Routing Pro. 
aurp            387/udp    ;Appletalk Update-Based Routing Pro. 
;                          ;Chris Ranch <cranch@novell.com> 
unidata-ldm     388/tcp    ;Unidata LDM Version 4 
unidata-ldm     388/udp    ;Unidata LDM Version 4 
;                          ;Glenn Davis <davis@unidata.ucar.edu> 
ldap            389/tcp    ;Lightweight Directory Access Protocol 
ldap            389/udp    ;Lightweight Directory Access Protocol 
;                          ;Tim Howes <Tim.Howes@terminator.cc.umich.edu> 
uis             390/tcp    ;UIS 
uis             390/udp    ;UIS 
;                          ;Ed Barron <---none---> 
synotics-relay  391/tcp    ;SynOptics SNMP Relay Port 
synotics-relay  391/udp    ;SynOptics SNMP Relay Port 
synotics-broker 392/tcp    ;SynOptics Port Broker Port 
synotics-broker 392/udp    ;SynOptics Port Broker Port 
;                          ;Illan Raab <iraab@synoptics.com> 
dis             393/tcp    ;Data Interpretation System 
dis             393/udp    ;Data Interpretation System 
;                          ;Paul Stevens <pstevens@chinacat.Metaphor.COM> 
embl-ndt        394/tcp    ;EMBL Nucleic Data Transfer 
embl-ndt        394/udp    ;EMBL Nucleic Data Transfer 
;                          ;Peter Gad <peter@bmc.uu.se> 
netcp           395/tcp    ;NETscout Control Protocol 
netcp           395/udp    ;NETscout Control Protocol 
;                          ;Anil Singhal <---none---> 
netware-ip      396/tcp    ;Novell Netware over IP 
netware-ip      396/udp    ;Novell Netware over IP 
mptn            397/tcp    ;Multi Protocol Trans. Net. 
mptn            397/udp    ;Multi Protocol Trans. Net. 
;                          ;Soumitra Sarkar <sarkar@vnet.ibm.com> 
kryptolan       398/tcp    ;Kryptolan 
kryptolan       398/udp    ;Kryptolan 
;                          ;Peter de Laval <pdl@sectra.se> 
;               399/tcp    ;Unassigned 
;               399/udp    ;Unassigned 
work-sol        400/tcp    ;Workstation Solutions 
work-sol        400/udp    ;Workstation Solutions 
;                          ;Jim Ward <jimw@worksta.com> 
ups             401/tcp    ;Uninterruptible Power Supply 
ups             401/udp    ;Uninterruptible Power Supply 
;                          ;Guenther Seybold <gs@hrz.th-darmstadt.de> 
genie           402/tcp    ;Genie Protocol 
genie           402/udp    ;Genie Protocol 
;                          ;Mark Hankin <---none---> 
decap           403/tcp    ;decap 
decap           403/udp    ;decap 
nced            404/tcp    ;nced 
nced            404/udp    ;nced 
ncld            405/tcp    ;ncld 
ncld            405/udp    ;ncld 
;                          ;Richard Jones <---none---> 
imsp            406/tcp    ;Interactive Mail Support Protocol 
imsp            406/udp    ;Interactive Mail Support Protocol 
;                          ;John Myers <jgm+@cmu.edu> 
timbuktu        407/tcp    ;Timbuktu 
timbuktu        407/udp    ;Timbuktu 
;                          ;Marc Epard <marc@waygate.farallon.com> 
prm-sm          408/tcp    ;Prospero Resource Manager Sys. Man. 
prm-sm          408/udp    ;Prospero Resource Manager Sys. Man. 
prm-nm          409/tcp    ;Prospero Resource Manager Node Man. 
prm-nm          409/udp    ;Prospero Resource Manager Node Man. 
;                          ;B. Clifford Neuman <bcn@isi.edu> 
decladebug      410/tcp    ;DECLadebug Remote Debug Protocol 
decladebug      410/udp    ;DECLadebug Remote Debug Protocol 
;                          ;Anthony Berent <berent@rdgeng.enet.dec.com> 
rmt             411/tcp    ;Remote MT Protocol 
rmt             411/udp    ;Remote MT Protocol 
;                          ;Peter Eriksson <pen@lysator.liu.se> 
synoptics-trap  412/tcp    ;Trap Convention Port 
synoptics-trap  412/udp    ;Trap Convention Port 
;                          ;Illan Raab <iraab@synoptics.com> 
smsp            413/tcp    ;SMSP 
smsp            413/udp    ;SMSP 
infoseek        414/tcp    ;InfoSeek 
infoseek        414/udp    ;InfoSeek 
;                          ;Steve Kirsch <stk@frame.com> 
bnet            415/tcp    ;BNet 
bnet            415/udp    ;BNet 
;                          ;Jim Mertz <JMertz+RV09@rvdc.unisys.com> 
silverplatter   416/tcp    ;Silverplatter 
silverplatter   416/udp    ;Silverplatter 
;                          ;Peter Ciuffetti <petec@silverplatter.com> 
onmux           417/tcp    ;Onmux 
onmux           417/udp    ;Onmux 
;                          ;Stephen Hanna <hanna@world.std.com> 
hyper-g         418/tcp    ;Hyper-G 
hyper-g         418/udp    ;Hyper-G 
;                          ;Frank Kappe <fkappe@iicm.tu-graz.ac.at> 
ariel1          419/tcp    ;Ariel 
ariel1          419/udp    ;Ariel 
;                          ;Jonathan Lavigne <BL.JPL@RLG.Stanford.EDU> 
smpte           420/tcp    ;SMPTE 
smpte           420/udp    ;SMPTE 
;                          ;Si Becker <71362.22@CompuServe.COM> 
ariel2          421/tcp    ;Ariel 
ariel2          421/udp    ;Ariel 
ariel3          422/tcp    ;Ariel 
ariel3          422/udp    ;Ariel 
;                          ;Jonathan Lavigne  <BL.JPL@RLG.Stanford.EDU> 
opc-job-start   423/tcp    ;IBM Operations Planning and Control Start 
opc-job-start   423/udp    ;IBM Operations Planning and Control Start 
opc-job-track   424/tcp    ;IBM Operations Planning and Control Track 
opc-job-track   424/udp    ;IBM Operations Planning and Control Track 
;                          ;Conny Larsson  <cocke@VNET.IBM.COM> 
icad-el         425/tcp    ;ICAD 
icad-el         425/udp    ;ICAD 
;                          ;Larry Stone  <lcs@icad.com> 
smartsdp        426/tcp    ;smartsdp 
smartsdp        426/udp    ;smartsdp 
;                          ;Alexander Dupuy <dupuy@smarts.com> 
svrloc          427/tcp    ;Server Location 
svrloc          427/udp    ;Server Location 
;                          ;<veizades@ftp.com> 
ocs_cmu         428/tcp    ;OCS_CMU 
ocs_cmu         428/udp    ;OCS_CMU 
ocs_amu         429/tcp    ;OCS_AMU 
ocs_amu         429/udp    ;OCS_AMU 
;                          ;Florence Wyman <wyman@peabody.plk.af.mil> 
utmpsd          430/tcp    ;UTMPSD 
utmpsd          430/udp    ;UTMPSD 
utmpcd          431/tcp    ;UTMPCD 
utmpcd          431/udp    ;UTMPCD 
iasd            432/tcp    ;IASD 
iasd            432/udp    ;IASD 
;                          ;Nir Baroz <nbaroz@encore.com> 
nnsp            433/tcp    ;NNSP 
nnsp            433/udp    ;NNSP 
;                          ;Rob Robertson <rob@gangrene.berkeley.edu> 
mobileip-agent  434/tcp    ;MobileIP-Agent 
mobileip-agent  434/udp    ;MobileIP-Agent 
mobilip-mn      435/tcp    ;MobilIP-MN 
mobilip-mn      435/udp    ;MobilIP-MN 
;                          ;Kannan Alagappan <kannan@sejour.lkg.dec.com> 
dna-cml         436/tcp    ;DNA-CML 
dna-cml         436/udp    ;DNA-CML 
;                          ;Dan Flowers <flowers@smaug.lkg.dec.com> 
comscm          437/tcp    ;comscm 
comscm          437/udp    ;comscm 
;                          ;Jim Teague <teague@zso.dec.com> 
dsfgw           438/tcp    ;dsfgw 
dsfgw           438/udp    ;dsfgw 
;                          ;Andy McKeen <mckeen@osf.org> 
dasp            439/tcp    ;dasp      Thomas Obermair 
dasp            439/udp    ;dasp      tommy@inlab.m.eunet.de 
;                          ;Thomas Obermair <tommy@inlab.m.eunet.de> 
sgcp            440/tcp    ;sgcp 
sgcp            440/udp    ;sgcp 
;                          ;Marshall Rose <mrose@dbc.mtview.ca.us> 
decvms-sysmgt   441/tcp    ;decvms-sysmgt 
decvms-sysmgt   441/udp    ;decvms-sysmgt 
;                          ;Lee Barton <barton@star.enet.dec.com> 
cvc_hostd       442/tcp    ;cvc_hostd 
cvc_hostd       442/udp    ;cvc_hostd 
;                          ;Bill Davidson <billd@equalizer.cray.com> 
https           443/tcp    ;https  MCom 
https           443/udp    ;https  MCom 
;                          ;Kipp E.B. Hickman <kipp@mcom.com> 
snpp            444/tcp    ;Simple Network Paging Protocol 
snpp            444/udp    ;Simple Network Paging Protocol 
;                          ;[RFC1568] 
microsoft-ds    445/tcp    ;Microsoft-DS 
microsoft-ds    445/udp    ;Microsoft-DS 
;                          ;Arnold Miller <arnoldm@microsoft.com> 
ddm-rdb         446/tcp    ;DDM-RDB 
ddm-rdb         446/udp    ;DDM-RDB 
ddm-dfm         447/tcp    ;DDM-RFM 
ddm-dfm         447/udp    ;DDM-RFM 
ddm-byte        448/tcp    ;DDM-BYTE 
ddm-byte        448/udp    ;DDM-BYTE 
;                          ;Jan David Fisher <jdfisher@VNET.IBM.COM> 
as-servermap    449/tcp    ;AS Server Mapper 
as-servermap    449/udp    ;AS Server Mapper 
;                          ;Barbara Foss <BGFOSS@rchvmv.vnet.ibm.com> 
tserver         450/tcp    ;TServer 
tserver         450/udp    ;TServer 
;                          ;Harvey S. Schultz <hss@mtgzfs3.mt.att.com> 
;               451-511    ;Unassigned 
exec            512/tcp    ;remote process execution; 
;                          ;authentication performed using 
;                          ;passwords and UNIX loppgin names 
biff            512/udp    ;used by mail system to notify users 
;                          ;of new mail received; currently 
;                          ;receives messages only from 
;                          ;processes on the same machine 
login           513/tcp    ;remote login a la telnet; 
;                          ;automatic authentication performed 
;                          ;based on priviledged port numbers 
;                          ;and distributed data bases which 
;                          ;identify "authentication domains" 
who             513/udp    ;maintains data bases showing who's 
;                          ;logged in to machines on a local 
;                          ;net and the load average of the 
;                          ;machine 
cmd             514/tcp    ;like exec, but automatic 
;                          ;authentication is performed as for 
;                          ;login server 
syslog          514/udp    ; 
printer         515/tcp    ;spooler 
printer         515/udp    ;spooler 
;               516/tcp    ;Unassigned 
;               516/udp    ;Unassigned 
talk            517/tcp    ;like tenex link, but across 
;                          ;machine - unfortunately, doesn't 
;                          ;use link protocol (this is actually 
;                          ;just a rendezvous port from which a 
;                          ;tcp connection is established) 
talk            517/udp    ;like tenex link, but across 
;                          ;machine - unfortunately, doesn't 
;                          ;use link protocol (this is actually 
;                          ;just a rendezvous port from which a 
                           ;tcp connection is established) 
ntalk           518/tcp    ; 
ntalk           518/udp    ; 
utime           519/tcp    ;unixtime 
utime           519/udp    ;unixtime 
efs             520/tcp    ;extended file name server 
router          520/udp    ;local routing process (on site); 
;                          ;uses variant of Xerox NS routing 
;                          ;information protocol 
;               521-524    ;Unassigned 
timed           525/tcp    ;timeserver 
timed           525/udp    ;timeserver 
tempo           526/tcp    ;newdate 
tempo           526/udp    ;newdate 
;               527-529    ;Unassigned 
courier         530/tcp    ;rpc 
courier         530/udp    ;rpc 
conference      531/tcp    ;chat 
conference      531/udp    ;chat 
netnews         532/tcp    ;readnews 
netnews         532/udp    ;readnews 
netwall         533/tcp    ;for emergency broadcasts 
netwall         533/udp    ;for emergency broadcasts 
;               534-538    ;Unassigned 
apertus-ldp     539/tcp    ;Apertus Technologies Load Determination 
apertus-ldp     539/udp    ;Apertus Technologies Load Determination 
uucp            540/tcp    ;uucpd 
uucp            540/udp    ;uucpd 
uucp-rlogin     541/tcp    ;uucp-rlogin  Stuart Lynne 
uucp-rlogin     541/udp    ;uucp-rlogin  sl@wimsey.com 
;               542/tcp    ;Unassigned 
;               542/udp    ;Unassigned 
klogin          543/tcp    ;
klogin          543/udp    ;
kshell          544/tcp    ;krcmd 
kshell          544/udp    ;krcmd 
;               545-549    ;Unassigned 
new-rwho        550/tcp    ;new-who 
new-rwho        550/udp    ;new-who 
;               551-555    ;Unassigned 
dsf             555/tcp    ; 
dsf             555/udp    ; 
remotefs        556/tcp    ;rfs server 
remotefs        556/udp    ;rfs server 
;               557-559    ;Unassigned 
rmonitor        560/tcp    ;rmonitord 
rmonitor        560/udp    ;rmonitord 
monitor         561/tcp    ; 
monitor         561/udp    ;
chshell         562/tcp    ;chcmd 
chshell         562/udp    ;chcmd 
;               563/tcp    ;Unassigned 
;               563/udp    ;Unassigned 
9pfs            564/tcp    ;plan 9 file service 
9pfs            564/udp    ;plan 9 file service 
whoami          565/tcp    ;whoami 
whoami          565/udp    ;whoami 
;               566-569    ;Unassigned 
meter           570/tcp    ;demon 
meter           570/udp    ;demon 
meter           571/tcp    ;udemon 
meter           571/udp    ;udemon 
;               572-599    ;Unassigned 
ipcserver       600/tcp    ;Sun IPC server 
ipcserver       600/udp    ;Sun IPC server 
nqs             607/tcp    ;nqs 
nqs             607/udp    ;nqs 
urm             606/tcp    ;Cray Unified Resource Manager 
urm             606/udp    ;Cray Unified Resource Manager 
;                          ;Bill Schiefelbein <schief@aspen.cray.com> 
sift-uft        608/tcp    ;Sender-Initiated/Unsolicited File Transfer 
sift-uft        608/udp    ;Sender-Initiated/Unsolicited File Transfer 
;                          ;Rick Troth <troth@rice.edu> 
npmp-trap       609/tcp    ;npmp-trap 
npmp-trap       609/udp    ;npmp-trap 
npmp-local      610/tcp    ;npmp-local 
npmp-local      610/udp    ;npmp-local 
npmp-gui        611/tcp    ;npmp-gui 
npmp-gui        611/udp    ;npmp-gui 
;                          ;John Barnes <jbarnes@crl.com> 
ginad           634/tcp    ;ginad 
ginad           634/udp    ;ginad 
;                          Mark Crother <mark@eis.calstate.edu> 
mdqs            666/tcp    ;
mdqs            666/udp    ;
doom            666/tcp    ;doom Id Software 
doom            666/tcp    ;doom Id Software 
;                          ;<ddt@idcube.idsoftware.com> 
elcsd           704/tcp    ;errlog copy/server daemon 
elcsd           704/udp    ;errlog copy/server daemon 
entrustmanager  709/tcp    ;EntrustManager 
entrustmanager  709/udp    ;EntrustManager 
;                          ;Peter Whittaker <pww@bnr.ca> 
netviewdm1      729/tcp    ;IBM NetView DM/6000 Server/Client 
netviewdm1      729/udp    ;IBM NetView DM/6000 Server/Client 
netviewdm2      730/tcp    ;IBM NetView DM/6000 send/tcp 
netviewdm2      730/udp    ;IBM NetView DM/6000 send/tcp 
netviewdm3      731/tcp    ;IBM NetView DM/6000 receive/tcp 
netviewdm3      731/udp    ;IBM NetView DM/6000 receive/tcp 
;                          ;Philippe Binet  (phbinet@vnet.IBM.COM) 
netgw           741/tcp    ;netGW 
netgw           741/udp    ;netGW 
netrcs          742/tcp    ;Network based Rev. Cont. Sys. 
netrcs          742/udp    ;Network based Rev. Cont. Sys. 
;                          ;Gordon C. Galligher <gorpong@ping.chi.il.us> 
flexlm          744/tcp    ;Flexible License Manager 
flexlm          744/udp    ;Flexible License Manager 
;                          ;Matt Christiano 
;                          ;<globes@matt@oliveb.atc.olivetti.com> 
fujitsu-dev     747/tcp    ;Fujitsu Device Control 
fujitsu-dev     747/udp    ;Fujitsu Device Control 
ris-cm          748/tcp    ;Russell Info Sci Calendar Manager 
ris-cm          748/udp    ;Russell Info Sci Calendar Manager 
kerberos-adm    749/tcp    ;kerberos administration 
kerberos-adm    749/udp    ;kerberos administration 
rfile           750/tcp    ;
loadav          750/udp    ;
pump            751/tcp    ;
pump            751/udp    ;
qrh             752/tcp    ;
qrh             752/udp    ;
rrh             753/tcp    ;
rrh             753/udp    ;
tell            754/tcp    ; send 
tell            754/udp    ; send 
nlogin          758/tcp    ;
nlogin          758/udp    ;
con             759/tcp    ;
con             759/udp    ;
ns              760/tcp    ;
ns              760/udp    ;
rxe             761/tcp    ;
rxe             761/udp    ;
quotad          762/tcp    ;
quotad          762/udp    ;
cycleserv       763/tcp    ;
cycleserv       763/udp    ;
omserv          764/tcp    ;
omserv          764/udp    ;
webster         765/tcp    ;
webster         765/udp    ;
phonebook       767/tcp    ;phone 
phonebook       767/udp    ;phone 
vid             769/tcp    ;
vid             769/udp    ;
cadlock         770/tcp    ;
cadlock         770/udp    ;
rtip            771/tcp    ;
rtip            771/udp    ;
cycleserv2      772/tcp    ;
cycleserv2      772/udp    ;
submit          773/tcp    ;
notify          773/udp    ;
rpasswd         774/tcp    ;
acmaint_dbd     774/udp    ;
entomb          775/tcp    ;
acmaint_transd  775/udp    ;
wpages          776/tcp    ;
wpages          776/udp    ;
wpgs            780/tcp    ;
wpgs            780/udp    ;
concert         786/tcp    ;  Concert 
concert         786/udp    ;  Concert 
;                          ;  Josyula R. Rao <jrrao@watson.ibm.com> 
mdbs_daemon     800/tcp    ;
mdbs_daemon     800/udp    ;
device          801/tcp    ;
device          801/udp    ;
xtreelic        996/tcp    ;   Central Point Software 
xtreelic        996/udp    ;   Central Point Software 
;                          ;   Dale Cabell <dacabell@smtp.xtree.com> 
maitrd          997/tcp    ;
maitrd          997/udp    ;
busboy          998/tcp    ;
puparp          998/udp    ;
garcon          999/tcp    ;
applix          999/udp    ;   Applix ac 
puprouter       999/tcp    ;
puprouter       999/udp    ;
cadlock         1000/tcp   ;
ock             1000/udp   ;
                1023/tcp   ;   Reserved 
                1024/udp   ;   Reserved 
;                          ;   IANA <iana@isi.edu> 



;
;----------------------------------------------------------------------------
; End of file:	SERVICES.SH
;----------------------------------------------------------------------------
