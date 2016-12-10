/*
 * bs1770gain.c
 * Copyright (C) 2014 Peter Belkner <pbelkner@users.sf.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2.0 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301  USA
 */
#include <bs1770gain_priv.h>
#include <getopt.h>
#if defined (_WIN32) // [
#include <fcntl.h>
//#define W_OPTION_UTF16
#if defined (PACKAGE_VERSION) // [
#define W_PACKAGE_VERSION PBU_WIDEN(PACKAGE_VERSION)
#endif // ]
#if defined (PACKAGE_URL) // [
#define W_PACKAGE_URL PBU_WIDEN(PACKAGE_URL)
#endif // ]
#define FPRINTF(stream,format) fwprintf(stream,L##format)
#else // ] [
#define FPRINTF(stream,format) fprintf(stream,format)
#endif // ]

///////////////////////////////////////////////////////////////////////////////
#define CLOCKS_PER_MILLIS (CLOCKS_PER_SEC/1000)

#define BS1770GAIN_AGGREGATE_METHOD ( \
  FFSOX_AGGREGATE_MOMENTARY_MAXIMUM \
  &FFSOX_AGGREGATE_MOMENTARY_MEAN \
  &FFSOX_AGGREGATE_SHORTTERM_MAXIMUM \
  &FFSOX_AGGREGATE_SHORTTERM_MEAN \
)

#define BS1770GAIN_AGGREGATE_RANGE_PEAK ( \
  FFSOX_AGGREGATE_MOMENTARY_RANGE \
  &FFSOX_AGGREGATE_SHORTTERM_RANGE \
  &FFSOX_AGGREGATE_SAMPLEPEAK \
  &FFSOX_AGGREGATE_TRUEPEAK \
)

#define BS1770GAIN_OPTIONS_NO_METHOD(o) \
  (0==((o)->aggregate&BS1770GAIN_AGGREGATE_METHOD))
#define BS1770GAIN_OPTIONS_NO_RANGE_PEAK(o) \
  (0==((o)->aggregate&BS1770GAIN_AGGREGATE_RANGE_PEAK))

///////////////////////////////////////////////////////////////////////////////
#if defined (_WIN32) // [
void bs1770gain_usage(wchar_t **wargv, char **argv, int code)
#else // ] [
void bs1770gain_usage(char **argv, int code)
#endif // ]
{
#if defined (_WIN32) // [
#if defined (W_PACKAGE_VERSION) // [
  fwprintf(stderr,L"BS1770GAIN %s, Copyright (C) Peter Belkner 2014-2017.\n",
      W_PACKAGE_VERSION);
#endif // ]
#if defined (W_PACKAGE_URL) // [
  fwprintf(stderr,L"%s\n",W_PACKAGE_URL);
#endif // ]
  fwprintf(stderr,L"\n");
  fwprintf(stderr,L"Usage:  %s [options] <file/dir> [<file/dir> ...]\n",
      pbu_wbasename(wargv[0]));
  fwprintf(stderr,L"\n");
#else // ] [
#if defined (PACKAGE_VERSION) // [
  fprintf(stderr,"BS1770GAIN %s, Copyright (C) Peter Belkner 2014-2017.\n",
      PACKAGE_VERSION);
#endif // ]
#if defined (PACKAGE_URL) // [
  fprintf(stderr,"%s\n",PACKAGE_URL);
#endif // ]
  fprintf(stderr,"\n");
  fprintf(stderr,"Usage:  %s [options] <file/dir> [<file/dir> ...]\n",
      pbu_basename(argv[0]));
  fprintf(stderr,"\n");
#endif // ]
  FPRINTF(stderr,"Options:\n");
  FPRINTF(stderr," -h,--help:  print this message and exit\n");
  /////////////////////////////////////////////////////////////////////////////
  FPRINTF(stderr," -i,--integrated:  calculate integrated loudness\n");
  FPRINTF(stderr," -s,--shorterm:  calculate maximum shortterm loudness\n");
  FPRINTF(stderr," -m,--momentary:  claculate maximum momentary loudness\n");
  FPRINTF(stderr," -r,--range:  calculate loudness range\n");
  FPRINTF(stderr," -p,--samplepeak:  calculate maximum sample peak\n");
  FPRINTF(stderr," -t,--truepeak:  calculate maximum true peak\n");
  FPRINTF(stderr," -b <timestamp>,--begin <timestamp>:  begin decoding at\n"
      "   timestamp (in microseconds, format: hh:mm:ss.mus)\n");
  FPRINTF(stderr," -d <duration>,--duration <duration>:  let decoding\n"
      "   last duration (in microseconds, format: hh:mm:ss.mus)\n");
  FPRINTF(stderr," -u <method>,--use <method>:  base replay gain calculation\n"
      "   on method (with respect to the -a/--apply and -o/--output\n"
      "   options),\n"
      "   methods:  \"integrated\" (default), \"shortterm\", or\n"
      "   \"momentary\"\n"
      "   experimental methods:  \"momentary-mean\" (same as\n"
      "   \"integrated\"), \"momentary-maximum\" (same as \"momentary\"),\n"
      "   \"shortterm-mean\", or \"shortterm-maximum\" (same as\n"
      "   \"shortterm\")\n");
  FPRINTF(stderr," -a:  apply the EBU/ATSC/RG album gain to the output\n"
      "   (in conjunction with the -o/--output option)\n");
  FPRINTF(stderr," -o <folder>,--output <folder>:  write RG tags\n"
      "   or apply the EBU/ATSC/RG gain, respectively,\n"
      "   and output to folder\n");
  FPRINTF(stderr," -f <file>,--file <file>:  write analysis to file\n");
  FPRINTF(stderr," -x,--extensions:  enable following extensions/defaults:\n"
      "   1) rename files according to TITLE tag\n"
      "   2) read metadata from per-folder CSV file \"folder.csv\"\n"
      "   3) copy file \"folder.jpg\" from source to destination\n"
      "      folder\n"
      "   4) automatically add the TRACK and DISC tags\n"
      "   5) calculate maximum true peak\n"
      "   6) convert to stereo\n");
#if defined (_WIN32) && defined (W_OPTION_UTF16) // [
  FPRINTF(stderr," -v,--utf-16:  use utf-16 for classical (i.e. non-xml)\n"
      "   output (may be useful for languages containing non-ascii symbols,\n"
      "   currently supresses displaying progress of analysis)\n");
#endif // ]
  FPRINTF(stderr," -l,--list:  print FFmpeg format/stream information\n");
  /////////////////////////////////////////////////////////////////////////////
  FPRINTF(stderr," --ebu:  calculate replay gain according to EBU R128\n"
      "   (-23.0 LUFS, default)\n");
  FPRINTF(stderr," --atsc:  calculate replay gain according to ATSC A/85\n"
      "   (-24.0 LUFS)\n");
  FPRINTF(stderr," --replaygain:  calculate replay gain according to\n"
      "   ReplayGain 2.0 (-18.0 LUFS)\n");
  FPRINTF(stderr," --track-tags:  write track tags\n");
  FPRINTF(stderr," --album-tags:  write album tags\n");
#if defined (BS1770GAIN_TAG_PREFIX) // [
#if 0 // [
  FPRINTF(stderr," --tag-prefix <prefix>:  use <prefix> as tag prefix\n"
      "   instead of \"REPLAYGAIN\"\n");
#else // ] [
  FPRINTF(stderr," --tag-prefix <prefix>:  instead of \"REPLAYGAIN\",\n"
      "   use <prefix> as RG tag prefix\n");
#endif // ]
#endif // ]
  FPRINTF(stderr," --unit <unit>:  write tags with <unit>\n");
  FPRINTF(stderr," --apply <q>:  apply the EBU/ATSC/RG gain to the output\n"
      "   (in conjunction with the -o/--output option),\n"
      "   q=0.0 (album gain) ... 1.0 (track gain),\n"
      "   default:  0.0 (album gain)\n");
  FPRINTF(stderr," --audio <index>:  select audio index (corresponds to\n"
      "   [0:<index>] in FFmpeg listing, cf. -l/--list option)\n");
  FPRINTF(stderr," --video <index>:  select video index (corresponds to\n"
      "   [0:<index>] in FFmpeg listing, cf. -l/--list option)\n");
  FPRINTF(stderr," --stereo:  convert to stereo\n");
  FPRINTF(stderr," --drc <float>:  set AC3 dynamic range compression (DRC)\n");
  FPRINTF(stderr," --extension <extension>:  enable extension out of\n"
      "   \"rename\":  rename files according to TITLE tag\n"
      "   \"csv\":  read metadata from per-folder CSV file \"folder.csv\"\n"
      "   \"jpg\":  copy file \"folder.jpg\" from source to destination\n"
      "      folder\n"
      "   \"tags\":  automatically add the TRACK and DISC tags\n");
  FPRINTF(stderr," --format <format>:  convert to format\n");
  FPRINTF(stderr," --loglevel <level>:  set loglevel,\n"
      "   level:  \"quiet\", \"panic\", \"fatal\", \"error\", \"warning\",\n"
      "   \"info\", \"verbose\", \"debug\"\n");
  FPRINTF(stderr," --xml:  print results in xml format\n");
  FPRINTF(stderr," --time:  print out duration of program invocation\n");
  FPRINTF(stderr," --norm <float>:  norm loudness to float.\n");
  //FPRINTF(stderr," --preamp <preamp>:\n");
  //FPRINTF(stderr," --stero:\n");
  //FPRINTF(stderr," --rg-tags:\n");
  //FPRINTF(stderr," --bwf-tags:\n");
  /////////////////////////////////////////////////////////////////////////////
  FPRINTF(stderr,"\n");
  FPRINTF(stderr,"Experimental options:\n");
  ////////
  FPRINTF(stderr,"1) momentary block\n");
  FPRINTF(stderr," --momentary-mean:  calculate mean loudness based on\n"
      "   momentary block (same as --integrated)\n");
  FPRINTF(stderr," --momentary-maximum:  calculate maximum loudness based\n"
      "   on momentary block (same as --momentary)\n");
  FPRINTF(stderr," --momentary-range:  calculate loudness range based on\n"
      "   momentary block\n");
  FPRINTF(stderr," --momentary-length <ms>:  length of momentary block\n"
      "   in milliseconds (default: 400)\n");
  FPRINTF(stderr," --momentary-overlap <percent>:  overlap of momentary\n"
      "   block in percent (default: 75)\n");
  FPRINTF(stderr," --momentary-mean-gate <gate>:  silence gate for mean\n"
      "   measurement of momentary block (default: -10.0)\n");
  FPRINTF(stderr," --momentary-range-gate <gate>:  silence gate for range\n"
      "   measurement of momentary block (default: -20.0)\n");
  FPRINTF(stderr," --momentary-range-lower-bound <float>:  lower bound for\n"
      "   range measurement of momentary block (default: 0.1)\n");
  FPRINTF(stderr," --momentary-range-upper-bound <float>:  upper bound for\n"
      "   range measurement of momentary block (default: 0.95)\n");
  ////////
  FPRINTF(stderr,"2) shortterm block\n");
  FPRINTF(stderr," --shortterm-mean:  calculate mean loudness based on\n"
      "   shortterm block\n");
  FPRINTF(stderr," --shortterm-maximum:  calculate maximum loudness based\n"
      "   on shortterm block (same as --shortterm)\n");
  FPRINTF(stderr," --shortterm-range:  calculate loudness range based on\n"
      "   shortterm block (same as --range)\n");
  FPRINTF(stderr," --shortterm-length <ms>:  length of shortterm block\n"
      "   in milliseconds (default: 3000)\n");
  FPRINTF(stderr," --shortterm-overlap <percent>:  overlap of shortterm\n"
      "   block in percent (default: 67)\n");
  FPRINTF(stderr," --shortterm-mean-gate <gate>:  silence gate for mean\n"
      "   measurement of shortterm block (default: -10.0)\n");
  FPRINTF(stderr," --shortterm-range-gate <gate>:  silence gate for range\n"
      "   measurement of shortterm block (default: -20.0)\n");
  FPRINTF(stderr," --shortterm-range-lower-bound <float>:  lower bound for\n"
      "   range measurement of shortterm block (default: 0.1)\n");
  FPRINTF(stderr," --shortterm-range-upper-bound <float>:  upper bound for\n"
      "   range measurement of shortterm block (default: 0.95)\n");
  /////////////////////////////////////////////////////////////////////////////
  FPRINTF(stderr,"\n");
  FPRINTF(stderr,"Command line arguments can appear in any order.\n");
#if defined (_WIN32) // [
  free(argv);
#endif // ]
  exit(code);
}

///////////////////////////////////////////////////////////////////////////////
enum {
  ////////////////
  APPLY='z'+1,
  UNIT,
  DRC,
  FORMAT,
  LOGLEVEL,
  NORM,
  PREAMP,
  ////////////////
  TIME,
  EBU,
  EXTENSION,
  ATSC,
  AUDIO,
  VIDEO,
  STEREO,
  REPLAYGAIN,
  RG_TAGS,
  BWF_TAGS,
  TRACK_TAGS,
  ALBUM_TAGS,
  XML,
#if defined (BS1770GAIN_TAG_PREFIX) // {
  TAG_PREFIX,
#endif // }
  ////////////////
  MOMENTARY_RANGE,
  MOMENTARY_LENGTH,
  MOMENTARY_OVERLAP,
  MOMENTARY_MEAN_GATE,
  MOMENTARY_RANGE_GATE,
  MOMENTARY_RANGE_LOWER_BOUND,
  MOMENTARY_RANGE_UPPER_BOUND,
  SHORTTERM_MEAN,
  SHORTTERM_LENGTH,
  SHORTTERM_OVERLAP,
  SHORTTERM_MEAN_GATE,
  SHORTTERM_RANGE_GATE,
  SHORTTERM_RANGE_LOWER_BOUND,
  SHORTTERM_RANGE_UPPER_BOUND
};

static const char *bs1770gain_flags="b:d:f:o:u:ahlimprstx"
#if defined (_WIN32) && defined (W_OPTION_UTF16) // [
    "v"
#endif // ]
    ;

static struct option bs1770gain_opts[]={
  ////
  { "begin",required_argument,NULL,'b' },
  { "duration",required_argument,NULL,'d' },
  { "file",required_argument,NULL,'f' },
  { "output",required_argument,NULL,'o' },
  { "use",required_argument,NULL,'u' },
  ////
  { "help",no_argument,NULL,'h' },
  { "integrated",no_argument,NULL,'i' },
  { "list",no_argument,NULL,'l' },
  { "momentary",no_argument,NULL,'m' },
  { "range",no_argument,NULL,'r' },
  { "samplepeak",no_argument,NULL,'p' },
  { "shortterm",no_argument,NULL,'s' },
  { "truepeak",no_argument,NULL,'t' },
  { "extensions",no_argument,NULL,'x' },
#if defined (_WIN32) && defined (W_OPTION_UTF16) // [
  { "utf-16",no_argument,NULL,'v' },
#endif // ]
  ////
  { "apply",required_argument,NULL,APPLY },
  { "unit",required_argument,NULL,UNIT },
  { "audio",required_argument,NULL,AUDIO },
  { "drc",required_argument,NULL,DRC },
  { "extension",required_argument,NULL,EXTENSION },
  { "format",required_argument,NULL,FORMAT },
  { "loglevel",required_argument,NULL,LOGLEVEL },
  { "norm",required_argument,NULL,NORM },
  { "preamp",required_argument,NULL,PREAMP },
  { "video",required_argument,NULL,VIDEO },
  ////
  { "time",no_argument,NULL,TIME },
  { "ebu",no_argument,NULL,EBU },
  { "atsc",no_argument,NULL,ATSC },
  { "stereo",no_argument,NULL,STEREO },
  { "replaygain",no_argument,NULL,REPLAYGAIN },
  { "rg-tags",no_argument,NULL,RG_TAGS },
  { "bwf-tags",no_argument,NULL,BWF_TAGS },
  { "track-tags",no_argument,NULL,TRACK_TAGS },
  { "album-tags",no_argument,NULL,ALBUM_TAGS },
  { "xml",no_argument,NULL,XML },
#if defined (BS1770GAIN_TAG_PREFIX) // {
  { "tag-prefix",required_argument,NULL,TAG_PREFIX },
#endif // }
  ////
  { "momentary-mean",no_argument,NULL,'i' },
  { "momentary-maximum",no_argument,NULL,'m' },
  { "momentary-range",no_argument,NULL,
      MOMENTARY_RANGE },
  { "momentary-length",required_argument,NULL,
      MOMENTARY_LENGTH },
  { "momentary-overlap",required_argument,NULL,
      MOMENTARY_OVERLAP },
  { "momentary-mean-gate",required_argument,NULL,
      MOMENTARY_MEAN_GATE },
  { "momentary-range-gate",required_argument,NULL,
      MOMENTARY_RANGE_GATE },
  { "momentary-range-lower-bound",required_argument,NULL,
      MOMENTARY_RANGE_LOWER_BOUND },
  { "momentary-range-upper-bound",required_argument,NULL,
      MOMENTARY_RANGE_UPPER_BOUND },
  ////
  { "shortterm-mean",no_argument,NULL,
      SHORTTERM_MEAN },
  { "shortterm-maximum",no_argument,NULL,'s' },
  { "shortterm-range",no_argument,NULL,'r' },
  { "shortterm-length",required_argument,NULL,
      SHORTTERM_LENGTH },
  { "shortterm-overlap",required_argument,NULL,
      SHORTTERM_OVERLAP },
  { "shortterm-mean-gate",required_argument,NULL,
      SHORTTERM_MEAN_GATE },
  { "shortterm-range-gate",required_argument,NULL,
      SHORTTERM_RANGE_GATE },
  { "shortterm-range-lower-bound",required_argument,NULL,
      SHORTTERM_RANGE_LOWER_BOUND },
  { "shortterm-range-upper-bound",required_argument,NULL,
      SHORTTERM_RANGE_UPPER_BOUND },
  ////
  { NULL,0,NULL,0 }
};

///////////////////////////////////////////////////////////////////////////////
#if defined (_WIN32) // {
static char **bs1770gain_wargv2argv(int argc, wchar_t **wargv)
{
  size_t size;
  wchar_t **rp,**mp1;
  char **argv,**wp1,*wp2,*mp2;

  mp1=wargv+argc;
  size=0;

  for (rp=wargv;rp<mp1;++rp) {
    size+=WideCharToMultiByte(
      CP_UTF8,    // _In_      UINT    CodePage,
      0,          // _In_      DWORD   dwFlags,
      *rp,        // _In_      LPCWSTR lpWideCharStr,
      -1,         // _In_      int     cchWideChar,
      NULL,       // _Out_opt_ LPSTR   lpMultiByteStr,
      0,          // _In_      int     cbMultiByte,
      NULL,       // _In_opt_  LPCSTR  lpDefaultChar,
      NULL        // _Out_opt_ LPBOOL  lpUsedDefaultChar
    );
  }


  if (NULL==(argv=MALLOC((argc*sizeof *argv)+size))) {
    DMESSAGE("allocating argv");
    goto malloc;
  }

  wp1=argv;
  wp2=(char *)(argv+argc);
  mp2=wp2+size;

  for (rp=wargv;rp<mp1;++rp) {
    *wp1++=wp2;

    wp2+=WideCharToMultiByte(
      CP_UTF8,    // _In_      UINT    CodePage,
      0,          // _In_      DWORD   dwFlags,
      *rp,        // _In_      LPCWSTR lpWideCharStr,
      -1,         // _In_      int     cchWideChar,
      wp2,        // _Out_opt_ LPSTR   lpMultiByteStr,
      mp2-wp2,    // _In_      int     cbMultiByte,
      NULL,       // _In_opt_  LPCSTR  lpDefaultChar,
      NULL        // _Out_opt_ LPBOOL  lpUsedDefaultChar
    );
  }

  return argv;
malloc:
  return NULL;
}

int wmain(int argc, wchar_t **wargv)
#else // } {
int main(int argc, char **argv)
#endif // }
{
#if defined (_WIN32) // {
  char **argv;
#endif // }
  options_t options;
  FILE *f=stdout;
  tree_t root;
  char *fpath=NULL;
#if defined (_WIN32) // [
  wchar_t *wfpath=NULL;
#endif // ]
  char *odirname=NULL;
  int loglevel=AV_LOG_QUIET;
  double overlap;
  double t1,t2;
  int c;

#if defined (_WIN32) // [
#if 0 // [
  int i;
  wchar_t *warg;
#endif // ]
#if 0 // [
  CONSOLE_FONT_INFOEX consoleFontInfoEx;
  BOOL ret;
#if 0 // [
  HANDLE hConsole;

  hConsole=GetStdHandle(STD_OUTPUT_HANDLE),

  fprintf(stderr,"GetStdHandle(STD_OUTPUT_HANDLE) with %p\n",hConsole);
#endif // ]

  ZeroMemory(&consoleFontInfoEx,sizeof consoleFontInfoEx);
  consoleFontInfoEx.cbSize=sizeof consoleFontInfoEx;

  ret=GetCurrentConsoleFontEx(
    GetStdHandle(STD_OUTPUT_HANDLE),
    FALSE,
    &consoleFontInfoEx
  );

  if (ret)
    fprintf(stderr,"nFont: %ld\n",consoleFontInfoEx.nFont);
  else {
    fprintf(stderr,"GetCurrentConsoleFontEx returning with %d\n",ret);
    fprintf(stderr,"GetLastError(): %ld\n",GetLastError());
  }

  consoleFontInfoEx.nFont=CP_UTF8;

  ret=SetCurrentConsoleFontEx(
    GetStdHandle(STD_OUTPUT_HANDLE),
    FALSE,
    &consoleFontInfoEx
  );

  if (ret)
    fprintf(stderr,"nFont: %ld\n",consoleFontInfoEx.nFont);
  else {
    fprintf(stderr,"GetSurrentConsoleFontEx returning with %d\n",ret);
    fprintf(stderr,"GetLastError(): %ld\n",GetLastError());
  }
#endif // ]
#if 0 // [
// http://stackoverflow.com/questions/10882277/properly-print-utf8-characters-in-windows-console
#if 0 // [
_setmode(_fileno(stdout), _O_U8TEXT);
_setmode(_fileno(stderr), _O_U8TEXT);
#else // ] [
_setmode(_fileno(stdout), _O_U16TEXT);
_setmode(_fileno(stderr), _O_U16TEXT);
#endif // ]
#endif // ]

  if (NULL==(argv=bs1770gain_wargv2argv(argc,wargv))) {
    DMESSAGE("converting arguments");
    goto argv;
  }

#if 0 // [
for (i=0;i<argc;++i) {
  fwprintf(stderr,L"arg[%d]: \"%s\"\t",i,wargv[i]);
  fprintf(stderr,"\"%s\"\t\t",argv[i]);
  warg=pbu_s2w(argv[i]);
  fwprintf(stderr,L"\"%s\"\t\n",warg);
  FREE(warg);
}
fprintf(stderr,"\n");
#endif // ]
#endif // ]

  if (1==argc) {
#if defined (_WIN32) // [
    bs1770gain_usage(wargv,argv,-1);
#else // ] [
    bs1770gain_usage(argv,-1);
#endif // ]
  }

  TRACE_PUSH();

  //setlocale(LC_ALL,"C");
  memset(&options,0,sizeof options);
  options.unit="LU";
  options.audio=-1;
  options.video=-1;
  options.norm=-23.0;
  options.audio_ext="flac";
  options.video_ext="mkv";
#if defined (BS1770GAIN_TAG_PREFIX) // {
  options.tag_prefix="REPLAYGAIN";
#endif // }
#if defined (_WIN32) // [
  options.utf16=0;
#endif // ]

  options.momentary.ms=400.0;
  options.momentary.partition=4;
  options.momentary.mean_gate=-10.0;
  options.momentary.range_gate=-20.0;
  options.momentary.range_lower_bound=0.1;
  options.momentary.range_upper_bound=0.95;

  options.shortterm.ms=3000.0;
  options.shortterm.partition=3;
  options.shortterm.mean_gate=-10.0;
  options.shortterm.range_gate=-20.0;
  options.shortterm.range_lower_bound=0.1;
  options.shortterm.range_upper_bound=0.95;

  opterr=0;

  while (0<=(c=getopt_long(argc,argv,bs1770gain_flags,bs1770gain_opts,NULL))) {
    switch (c) {
    ///////////////////////////////////////////////////////////////////////////
    case '?':
      fprintf(stderr,"Error: Option \"%s\" not recognizsed.\n",argv[optind-1]);
      fprintf(stderr,"\n");
#if defined (_WIN32) // [
      bs1770gain_usage(wargv,argv,-1);
#else // ] [
      bs1770gain_usage(argv,-1);
#endif // ]
      break;
    /// with argument /////////////////////////////////////////////////////////
    case 'b':
      options.begin=bs1770gain_parse_time(optarg);
      break;
    case 'd':
      options.duration=bs1770gain_parse_time(optarg);
      break;
    case 'f':
      fpath=optarg;
      break;
    case 'o':
      odirname=optarg;
      break;
    case 'u':
      if (0==strcasecmp("integrated",optarg)
          ||0==strcasecmp("momentary-mean",optarg)) {
        options.aggregate|=FFSOX_AGGREGATE_MOMENTARY_MEAN;
        options.method=BS1770GAIN_METHOD_MOMENTARY_MEAN;
      }
      else if (0==strcasecmp("momentary",optarg)
          ||0==strcasecmp("momentary-maximum",optarg)) {
        options.aggregate|=FFSOX_AGGREGATE_MOMENTARY_MAXIMUM;
        options.method=BS1770GAIN_METHOD_MOMENTARY_MAXIMUM;
      }
      else if (0==strcasecmp("shortterm-mean",optarg)) {
        options.aggregate|=FFSOX_AGGREGATE_SHORTTERM_MEAN;
        options.method=BS1770GAIN_METHOD_SHORTTERM_MEAN;
      }
      else if (0==strcasecmp("shortterm",optarg)
          ||0==strcasecmp("shortterm-maximum",optarg)) {
        options.aggregate|=FFSOX_AGGREGATE_SHORTTERM_MAXIMUM;
        options.method=BS1770GAIN_METHOD_SHORTTERM_MAXIMUM;
      }
      else {
        fprintf(stderr,"Error: Method \"%s\" not recognized.\n",optarg);
        fprintf(stderr,"\n");
#if defined (_WIN32) // [
        bs1770gain_usage(wargv,argv,-1);
#else // ] [
        bs1770gain_usage(argv,-1);
#endif // ]
      }

      break;
    /// without argument //////////////////////////////////////////////////////
    case 'a':
      options.mode|=BS1770GAIN_MODE_APPLY;
      options.apply=0.0;
      break;
    case 'h':
#if defined (_WIN32) // [
      bs1770gain_usage(wargv,argv,0);
#else // ] [
      bs1770gain_usage(argv,0);
#endif // ]
      break;
    case 'l':
      options.dump=1;
      break;
    case 'i':
      options.aggregate|=FFSOX_AGGREGATE_MOMENTARY_MEAN;
      break;
    case 's':
      options.aggregate|=FFSOX_AGGREGATE_SHORTTERM_MAXIMUM;
      break;
    case 'm':
      options.aggregate|=FFSOX_AGGREGATE_MOMENTARY_MAXIMUM;
      break;
    case 'r':
      options.aggregate|=FFSOX_AGGREGATE_SHORTTERM_RANGE;
      break;
    case 'p':
      options.aggregate|=FFSOX_AGGREGATE_SAMPLEPEAK;
      break;
    case 't':
      options.aggregate|=FFSOX_AGGREGATE_TRUEPEAK;
      break;
    case 'x':
      options.extensions=BS1770GAIN_EXTENSION_ALL;
      options.aggregate|=FFSOX_AGGREGATE_TRUEPEAK;
      options.stereo=1;
      break;
#if defined (_WIN32) && defined (W_OPTION_UTF16) // [
    case 'v':
      options.utf16=1;
      break;
#endif // ]
    /// without flag //////////////////////////////////////////////////////////
    case AUDIO:
      options.audio=atoi(optarg);
      break;
    case VIDEO:
      options.video=atoi(optarg);
      break;
    case APPLY:
      options.mode|=BS1770GAIN_MODE_APPLY;
      options.apply=atof(optarg);
      break;
    case UNIT:
      options.unit=optarg;
      break;
    case DRC:
      options.drc=atof(optarg);
      break;
    case EXTENSION:
      if (0==strcasecmp("rename",optarg))
        options.extensions|=EXTENSION_RENAME;
      else if (0==strcasecmp("csv",optarg))
        options.extensions|=EXTENSION_CSV;
      else if (0==strcasecmp("jpg",optarg))
        options.extensions|=EXTENSION_JPG;
      else if (0==strcasecmp("tags",optarg))
        options.extensions|=EXTENSION_TAGS;
      else {
#if defined (_WIN32) // [
        bs1770gain_usage(wargv,argv,-1);
#else // ] [
        bs1770gain_usage(argv,-1);
#endif // ]
      }
      break;
#if defined (BS1770GAIN_TAG_PREFIX) // {
    case TAG_PREFIX:
      options.tag_prefix=optarg;
      break;
#endif // }
    case FORMAT:
      options.format=optarg;
      break;
    case LOGLEVEL:
      if (0==strcasecmp("quiet",optarg))
        loglevel=AV_LOG_QUIET;
      else if (0==strcasecmp("panic",optarg))
        loglevel=AV_LOG_PANIC;
      else if (0==strcasecmp("fatal",optarg))
        loglevel=AV_LOG_FATAL;
      else if (0==strcasecmp("error",optarg))
        loglevel=AV_LOG_ERROR;
      else if (0==strcasecmp("warning",optarg))
        loglevel=AV_LOG_WARNING;
      else if (0==strcasecmp("info",optarg))
        loglevel=AV_LOG_INFO;
      else if (0==strcasecmp("verbose",optarg))
        loglevel=AV_LOG_VERBOSE;
      else if (0==strcasecmp("debug",optarg))
        loglevel=AV_LOG_DEBUG;
      else {
#if defined (_WIN32) // [
        bs1770gain_usage(wargv,argv,-1);
#else // ] [
        bs1770gain_usage(argv,-1);
#endif // ]
      }
      break;
    case NORM:
      options.norm=atof(optarg);
      break;
    case PREAMP:
      options.preamp=atof(optarg);
      break;
    /// without flag //////////////////////////////////////////////////////////
    case ATSC:
      options.norm=-24.0;
      break;
    case EBU:
      options.norm=-23.0;
      break;
    case REPLAYGAIN:
      options.norm=-18.0;
      options.unit="dB";
      break;
    case RG_TAGS:
      options.mode|=BS1770GAIN_MODE_RG_TAGS;
      break;
    case BWF_TAGS:
      options.mode|=BS1770GAIN_MODE_BWF_TAGS;
      break;
    case TRACK_TAGS:
      options.mode|=BS1770GAIN_MODE_TRACK_TAGS;
      break;
    case ALBUM_TAGS:
      options.mode|=BS1770GAIN_MODE_ALBUM_TAGS;
      break;
    case XML:
      options.xml=1;
      break;
    case TIME:
      options.time=1;
      break;
    case STEREO:
      options.stereo=1;
      break;
    ///////////////////////////////////////////////////////////////////////////
    case MOMENTARY_RANGE:
      options.aggregate|=FFSOX_AGGREGATE_MOMENTARY_RANGE;
      break;
    ////////
    case MOMENTARY_LENGTH:
      options.momentary.ms=atof(optarg);
      break;
    case MOMENTARY_OVERLAP:
      overlap=atof(optarg);

      if (0.0<=overlap&&overlap<100.0)
        options.momentary.partition=floor(100.0/(100.0-overlap)+0.5);
      else {
        FPRINTF(stderr,"Error: Overlap out of range [0..100).\n");
        FPRINTF(stderr,"\n");
#if defined (_WIN32) // [
        bs1770gain_usage(wargv,argv,-1);
#else // ] [
        bs1770gain_usage(argv,-1);
#endif // ]
      }

      break;
    case MOMENTARY_MEAN_GATE:
      options.momentary.mean_gate=atof(optarg);
      break;
    case MOMENTARY_RANGE_GATE:
      options.momentary.range_gate=atof(optarg);
      break;
    case MOMENTARY_RANGE_LOWER_BOUND:
      options.momentary.range_lower_bound=atof(optarg);
      break;
    case MOMENTARY_RANGE_UPPER_BOUND:
      options.momentary.range_upper_bound=atof(optarg);
      break;
    ////////
    case SHORTTERM_MEAN:
      options.aggregate|=FFSOX_AGGREGATE_SHORTTERM_MEAN;
      break;
    ////////
    case SHORTTERM_LENGTH:
      options.shortterm.ms=atof(optarg);
      break;
    case SHORTTERM_OVERLAP:
      overlap=atof(optarg);

      if (0.0<=overlap&&overlap<100.0)
        options.shortterm.partition=floor(100.0/(100.0-overlap)+0.5);
      else {
        FPRINTF(stderr,"Error: Overlap out of range [0..100).\n");
        FPRINTF(stderr,"\n");
#if defined (_WIN32) // [
        bs1770gain_usage(wargv,argv,-1);
#else // ] [
        bs1770gain_usage(argv,-1);
#endif // ]
      }

      break;
    case SHORTTERM_MEAN_GATE:
      options.shortterm.mean_gate=atof(optarg);
      break;
    case SHORTTERM_RANGE_GATE:
      options.shortterm.range_gate=atof(optarg);
      break;
    case SHORTTERM_RANGE_LOWER_BOUND:
      options.shortterm.range_lower_bound=atof(optarg);
      break;
    case SHORTTERM_RANGE_UPPER_BOUND:
      options.shortterm.range_upper_bound=atof(optarg);
      break;
    ///////////////////////////////////////////////////////////////////////////
    default:
      break;
    }
  }

  if (0!=options.method&&NULL==odirname) {
    FPRINTF(stderr,"Error: \"-u/--use\" requieres \"-o/--output\".\n");
    FPRINTF(stderr,"\n");
#if defined (_WIN32) // [
    bs1770gain_usage(wargv,argv,-1);
#else // ] [
    bs1770gain_usage(argv,-1);
#endif // ]
  }

  if (BS1770GAIN_IS_MODE_APPLY(options.mode)&&NULL!=options.format) {
    fprintf(stderr,"Warning: Format \"%s\" potentially not available.\n",
        options.format);
    options.format=NULL;
  }

  if (options.momentary.partition<1||options.shortterm.partition<1) {
#if defined (_WIN32) // [
    bs1770gain_usage(wargv,argv,-1);
#else // ] [
    bs1770gain_usage(argv,-1);
#endif // ]
  }

  if (options.momentary.range_lower_bound<=0.0
      ||options.momentary.range_upper_bound
          <=options.momentary.range_lower_bound
      ||1.0<=options.momentary.range_upper_bound
      ||options.shortterm.range_lower_bound<=0.0
      ||options.shortterm.range_upper_bound
          <=options.shortterm.range_lower_bound
      ||1.0<=options.shortterm.range_upper_bound) {
    FPRINTF(stderr,"Error: Range bounds out of range "
        " 0.0 < lower < upper < 1.0.\n");
    FPRINTF(stderr,"\n");
#if defined (_WIN32) // [
    bs1770gain_usage(wargv,argv,-1);
#else // ] [
    bs1770gain_usage(argv,-1);
#endif // ]
  }

  if (!BS1770GAIN_IS_MODE_APPLY(options.mode)) {
    if (!BS1770GAIN_IS_MODE_RG_BWF_TAGS(options.mode))
      options.mode|=BS1770GAIN_MODE_RG_TAGS;

    if (!BS1770GAIN_IS_MODE_TRACK_ALBUM_TAGS(options.mode))
      options.mode|=BS1770GAIN_MODE_TRACK_ALBUM_TAGS;
  }

  if (BS1770GAIN_OPTIONS_NO_METHOD(&options)) {
    if (NULL!=odirname||BS1770GAIN_OPTIONS_NO_RANGE_PEAK(&options))
      options.aggregate|=FFSOX_AGGREGATE_MOMENTARY_MEAN;
  }

#if defined (_WIN32) // [
#if defined (W_OPTION_UTF16) // [
  options.utf16&=!options.xml;
#else // ] [
  options.utf16=!options.xml;
#endif // ]
#endif // ]

  // load the FFmpeg and SoX libraries from "bs1770gain-tools".
  if (ffsox_dynload("bs1770gain-tools")<0) {
    PBU_DMESSAGE("loading shared libraries");
    goto dynload;
  }

  av_register_all();
  sox_init();

  if (0==options.dump||av_log_get_level()<loglevel)
    av_log_set_level(loglevel);

  if (fpath) {
#if defined (_WIN32) // [
    if (NULL==(wfpath=pbu_s2w(fpath))) {
      fprintf(stderr,"Error converting file name to wide characters.\n");
      goto wfpath;
    }

    if (options.utf16) {
#if 0 // [
      // https://msdn.microsoft.com/en-us/library/yeby3zcb.aspx
      // seems not to work with mingw
      if (NULL==(f=fopen(fpath,"w,css=UNICODE"))) {
        fprintf(stderr,"Error opening file \"%s\".\n",fpath);
        goto file;
      }
#else // ] [
      // open in binary mode, write explicit BOM,
      // then switch to respective text mode
      if (NULL==(f=_wfopen(wfpath,L"wb"))) {
        fwprintf(stderr,L"Error opening file \"%s\".\n",wfpath);
        goto file;
      }

      // write BOM
      // https://en.wikipedia.org/wiki/Byte_order_mark#UTF-16
      fputc(0xff,f);
      fputc(0xfe,f);
      fflush(f);
      // switch to respective text mode, cf. below
#endif // ]
    }
    else if (NULL==(f=_wfopen(wfpath,L"w"))) {
      fwprintf(stderr,L"Error opening file \"%s\".\n",wfpath);
      goto file;
    }
#else // ] [
    if (NULL==(f=fopen(fpath,"w"))) {
      fprintf(stderr,"Error opening file \"%s\".\n",fpath);
      goto file;
    }
#endif // ]
  }

#if defined (_WIN32) // [
  if (options.utf16) {
    // http://stackoverflow.com/questions/10882277/properly-print-utf8-characters-in-windows-console
    _setmode(_fileno(f),_O_WTEXT);
  }
#endif // ]

  bs1770gain_tree_cli_init(&root,argc,argv,optind);

  if (options.xml)
    bs1770gain_print_xml(&options.p,f);
  else
    bs1770gain_print_classic(&options.p,f);

  options.p.vmt->session.head(&options.p);
  t1=clock();
  bs1770gain_tree_analyze(&root,odirname,&options);
  t2=clock();

  if (0==root.cli.count)
    fprintf(stderr,"Warning: No valid input files/folders given.\n");

  options.p.vmt->session.tail(&options.p);
  root.vmt->cleanup(&root);

  if (options.time)
    fprintf(stderr, "Duration: %.0f ms.\n",(t2-t1)/CLOCKS_PER_MILLIS);
// cleanup:
  sox_quit();
  // still reachable: 9,689 bytes in 51 blocks
  // TODO: Cleanup FFmpeg. But how?
dynload:
#if defined (_WIN32) // [
  if (NULL!=wfpath)
    fclose(f);
  else
#endif // ]
  if (NULL!=fpath)
    fclose(f);
file:
#if defined (_WIN32) // [
  if (NULL!=wfpath)
    FREE(wfpath);
wfpath:
#endif // ]
#if defined (_WIN32) // {
  FREE(argv);
argv:
#endif // }
  TRACE_POP();
  PBU_HEAP_PRINT();

  return 0;
}
