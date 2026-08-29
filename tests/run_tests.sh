#!/usr/bin/env bash
set -u

ROOT=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
FT=$ROOT/ft_ls

VERBOSE=0
KEEP=0
ONLY=""
MAX_DIFF=12

usage()
{
	echo "usage: run_tests.sh [--only MOTIF] [--verbose] [--keep]"
	echo
	echo "  --only MOTIF   ne lance que les categories dont le nom contient MOTIF"
	echo "  --verbose      affiche aussi les cas qui passent"
	echo "  --keep         garde le corpus au lieu de le detruire"
	echo
	echo "  GNU_LS=/chemin/vers/ls   force le ls de reference"
}

while [ $# -gt 0 ]; do
	case "$1" in
		--verbose|-v) VERBOSE=1 ;;
		--keep) KEEP=1 ;;
		--only) shift; ONLY="${1:-}" ;;
		--help|-h) usage; exit 0 ;;
		*) echo "option inconnue: $1" >&2; usage >&2; exit 2 ;;
	esac
	shift
done

RED=$'\033[31m'; GREEN=$'\033[32m'; YELLOW=$'\033[33m'
CYAN=$'\033[36m'; GRAY=$'\033[90m'; BOLD=$'\033[1m'; RESET=$'\033[0m'
if [ ! -t 1 ]; then RED=""; GREEN=""; YELLOW=""; CYAN=""; GRAY=""; BOLD=""; RESET=""; fi

if [ ! -x "$FT" ]; then
	echo "${RED}ft_ls introuvable ($FT), lance make d'abord${RESET}" >&2
	exit 2
fi

find_gnu_ls()
{
	local c
	for c in "${GNU_LS:-}" gls /opt/homebrew/opt/coreutils/libexec/gnubin/ls \
		/usr/local/opt/coreutils/libexec/gnubin/ls /bin/ls /usr/bin/ls; do
		[ -z "$c" ] && continue
		command -v "$c" >/dev/null 2>&1 || continue
		if "$c" --version 2>/dev/null | head -1 | grep -q coreutils; then
			command -v "$c"
			return 0
		fi
	done
	return 1
}

GNU=$(find_gnu_ls) || {
	echo "${RED}GNU ls introuvable.${RESET}" >&2
	echo "  macOS : brew install coreutils   (fournit gls)" >&2
	echo "  sinon : GNU_LS=/chemin/vers/ls $0" >&2
	exit 2
}
GNU_NAME=$(basename "$GNU")

if [ "$(uname -s)" != "Linux" ]; then
	echo "${YELLOW}attention${RESET} : $GNU est un coreutils compile sur $(uname -s)."
	echo "            la reference du projet est le ls de GNU sur glibc, et son"
	echo "            comportement differe ici (acl, locales). la CI fait foi."
	echo
fi

export LC_ALL=C
export TZ=UTC
unset LS_COLORS COLUMNS BLOCK_SIZE TIME_STYLE QUOTING_STYLE LS_BLOCK_SIZE
unset COLORTERM CLICOLOR
export TERM=xterm

IS_ROOT=0
[ "$(id -u)" = "0" ] && IS_ROOT=1

CORPUS=$(mktemp -d "${TMPDIR:-/tmp}/ft_ls_tests.XXXXXX")
cleanup()
{
	if [ "$KEEP" = "1" ]; then
		echo "${GRAY}corpus garde : $CORPUS${RESET}"
		return
	fi
	chmod -R u+rwX "$CORPUS" 2>/dev/null
	rm -rf "$CORPUS"
}
trap cleanup EXIT

HAVE_ACL=0

build_corpus()
{
	local d i n

	d=$CORPUS/types; mkdir -p "$d"
	: > "$d/vide"
	printf 'bonjour\n' > "$d/petit"
	head -c 4096 /dev/zero > "$d/moyen" 2>/dev/null
	head -c 300000 /dev/zero > "$d/gros" 2>/dev/null
	printf '#!/bin/sh\n' > "$d/script"; chmod 755 "$d/script"
	cp "$d/script" "$d/setuid"; chmod 4755 "$d/setuid" 2>/dev/null
	cp "$d/script" "$d/setgid"; chmod 2755 "$d/setgid" 2>/dev/null
	cp "$d/petit" "$d/lecture_seule"; chmod 444 "$d/lecture_seule"
	cp "$d/petit" "$d/aucun_droit"; chmod 000 "$d/aucun_droit"
	mkdir -p "$d/dossier" "$d/dossier_vide"
	: > "$d/dossier/dedans"
	mkdir -p "$d/sticky"; chmod 1777 "$d/sticky" 2>/dev/null
	mkdir -p "$d/ecrivable"; chmod 777 "$d/ecrivable" 2>/dev/null
	ln -s petit "$d/lien"
	ln -s dossier "$d/lien_dossier"
	ln -s nulle_part "$d/lien_casse"
	ln -s boucle_b "$d/boucle_a"; ln -s boucle_a "$d/boucle_b"
	ln -s soi_meme "$d/soi_meme"
	ln "$d/petit" "$d/dur1"; ln "$d/petit" "$d/dur2"
	mkfifo "$d/tube" 2>/dev/null
	: > "$d/.cache"
	: > "$d/.profil"
	touch -t 200001020304 "$d/vieux" 2>/dev/null || : > "$d/vieux"
	touch -t 202001020304 "$d/moins_vieux" 2>/dev/null || : > "$d/moins_vieux"
	: > "$d/recent"

	d=$CORPUS/ext; mkdir -p "$d"
	for i in a.c b.h c.o d.tar e.tar.gz f.jpg g.png h.mp3 i.txt j.md k. l \
		m.C n.Z o.zip p.sh q.py; do : > "$d/$i"; done
	chmod 755 "$d/p.sh"

	d=$CORPUS/tri; mkdir -p "$d"
	for i in 0 1 7 63 511 4095 100000; do
		head -c "$i" /dev/zero > "$d/taille_$i" 2>/dev/null || : > "$d/taille_$i"
	done
	for i in 1 2 3 4 5; do
		: > "$d/date_$i"
		touch -t "20200${i}0100${i}0" "$d/date_$i" 2>/dev/null
	done
	: > "$d/meme_a"; : > "$d/meme_b"; : > "$d/meme_c"
	touch -t 201501010000 "$d/meme_a" "$d/meme_b" "$d/meme_c" 2>/dev/null

	d=$CORPUS/larg; mkdir -p "$d"
	for i in $(seq 1 40); do
		n=$(printf "%0${i}d" "$i")
		: > "$d/${n:0:$i}"
	done
	d=$CORPUS/court; mkdir -p "$d"
	for i in a b c d e f g h i j k l m n o p q r s t u v w x y z; do : > "$d/$i"; done
	d=$CORPUS/long; mkdir -p "$d"
	for i in 1 2 3 4 5 6; do
		: > "$d/nom_vraiment_tres_long_pour_les_colonnes_numero_$i"
	done

	d=$CORPUS/masse; mkdir -p "$d"
	for i in $(seq 1 200); do : > "$d/fichier_$i"; done

	d=$CORPUS/arbre; mkdir -p "$d/a/b/c/d" "$d/a/b2" "$d/z" "$d/a/.cache"
	: > "$d/racine"; : > "$d/a/f_a"; : > "$d/a/b/f_b"
	: > "$d/a/b/c/f_c"; : > "$d/a/b/c/d/f_d"; : > "$d/a/.cache/f_h"
	ln -s ../.. "$d/a/b/remonte"
	ln -s "$d/a" "$d/z/vers_a"

	d=$CORPUS/noms; mkdir -p "$d"
	: > "$d/nom avec espaces"
	: > "$d/apostrophe's"
	: > "$d/double\"quote"
	: > "$d/anti\\slash"
	: > "$d/-tiret"
	: > "$d/--double-tiret"
	: > "$d/point.point.point"
	: > "$d/MAJUSCULE"
	: > "$d/minuscule"
	: > "$d/12chiffres"
	: > "$d/~tilde"
	: > "$d/point.virgule;"

	d=$CORPUS/droits; mkdir -p "$d"
	mkdir -p "$d/lisible_non_traversable"
	: > "$d/lisible_non_traversable/f1"
	: > "$d/lisible_non_traversable/f2"
	mkdir -p "$d/interdit"; : > "$d/interdit/f"
	mkdir -p "$d/ok"; : > "$d/ok/f"
	if [ "$IS_ROOT" = "0" ]; then
		chmod 400 "$d/lisible_non_traversable"
		chmod 000 "$d/interdit"
	fi

	d=$CORPUS/acl; mkdir -p "$d"
	for i in 1 2 3 4 5 6; do : > "$d/f$i"; done
	mkdir -p "$d/d1" "$d/d2"
	if chmod +a "everyone allow read" "$d/f2" 2>/dev/null; then
		HAVE_ACL=1
		chmod +a "everyone allow read" "$d/f5" "$d/d1" 2>/dev/null
	elif setfacl -m "u:$(id -un):r" "$d/f2" 2>/dev/null; then
		HAVE_ACL=1
		setfacl -m "u:$(id -un):r" "$d/f5" "$d/d1" 2>/dev/null
	fi

	mkdir -p "$CORPUS/rien"
}

TOTAL=0
FAILED=0
CAT=""
CAT_TOTAL=0
CAT_FAILED=0
declare -a REPORT=()

start_cat()
{
	end_cat
	CAT="$1"
	CAT_TOTAL=0
	CAT_FAILED=0
}

end_cat()
{
	[ -z "$CAT" ] && return
	local color=$GREEN mark="ok"
	if [ "$CAT_FAILED" != "0" ]; then color=$RED; mark="ECHEC"; fi
	REPORT+=("$(printf '  %-32s %5d cas   %s%4d ecart%s' \
		"$CAT" "$CAT_TOTAL" "$color" "$CAT_FAILED" "$RESET")")
	printf '%s%-32s%s %5d cas   %s%s%s\n' \
		"$CYAN" "$CAT" "$RESET" "$CAT_TOTAL" "$color" "$mark" "$RESET"
	CAT=""
}

skip_cat()
{
	[ -n "$ONLY" ] && [[ "$1" != *"$ONLY"* ]] && return 0
	return 1
}

normalise()
{
	local s=$1
	s=${s//ft_ls: /PROG: }
	s=${s//\'ft_ls --help\'/\'PROG --help\'}
	s=${s//$GNU: /PROG: }
	s=${s//$GNU_NAME: /PROG: }
	s=${s//\'$GNU --help\'/\'PROG --help\'}
	s=${s//\'$GNU_NAME --help\'/\'PROG --help\'}
	printf '%s' "$s"
}

report_diff()
{
	local label=$1 args=$2 rcf=$3 rcg=$4 gn=$5 ft=$6
	printf '  %sECART%s [%s] cwd=%s  (rc ft=%s gnu=%s)\n' \
		"$RED" "$RESET" "$args" "$label" "$rcf" "$rcg"
	diff <(printf '%s\n' "$gn") <(printf '%s\n' "$ft") \
		| head -n "$MAX_DIFF" | sed 's/^/      /' | cat -v
}

check()
{
	local cwd="$1"; shift
	local out_ft out_gn rc_ft rc_gn

	out_ft=$(cd "$cwd" && "$FT" "$@" 2>&1); rc_ft=$?
	out_gn=$(cd "$cwd" && "$GNU" "$@" 2>&1); rc_gn=$?
	out_ft=$(normalise "$out_ft")
	out_gn=$(normalise "$out_gn")

	TOTAL=$((TOTAL + 1))
	CAT_TOTAL=$((CAT_TOTAL + 1))
	if [ "$rc_ft" = "$rc_gn" ] && [ "$out_ft" = "$out_gn" ]; then
		[ "$VERBOSE" = "1" ] && printf '    %sok%s   %s\n' "$GRAY" "$RESET" "$*"
		return 0
	fi
	FAILED=$((FAILED + 1))
	CAT_FAILED=$((CAT_FAILED + 1))
	report_diff "${cwd#$CORPUS/}" "$*" "$rc_ft" "$rc_gn" "$out_gn" "$out_ft"
	return 1
}

check_env()
{
	local var="$1"; shift
	local cwd="$1"; shift
	local out_ft out_gn rc_ft rc_gn

	out_ft=$(cd "$cwd" && env $var "$FT" "$@" 2>&1); rc_ft=$?
	out_gn=$(cd "$cwd" && env $var "$GNU" "$@" 2>&1); rc_gn=$?
	out_ft=$(normalise "$out_ft")
	out_gn=$(normalise "$out_gn")

	TOTAL=$((TOTAL + 1))
	CAT_TOTAL=$((CAT_TOTAL + 1))
	if [ "$rc_ft" = "$rc_gn" ] && [ "$out_ft" = "$out_gn" ]; then
		[ "$VERBOSE" = "1" ] && printf '    %sok%s   %s %s\n' "$GRAY" "$RESET" "$var" "$*"
		return 0
	fi
	FAILED=$((FAILED + 1))
	CAT_FAILED=$((CAT_FAILED + 1))
	report_diff "${cwd#$CORPUS/}" "$var $*" "$rc_ft" "$rc_gn" "$out_gn" "$out_ft"
	return 1
}

FLAGS=(-a -A -C -c -d -f -F -g -G -h -i -l -n -o -p -R -r -s -S -t -u -U -x -X -1)
DOSSIERS=(types ext tri noms arbre court long rien)

cat_options_simples()
{
	skip_cat "options simples" && return
	start_cat "options simples"
	local f d
	for d in "${DOSSIERS[@]}"; do
		check "$CORPUS/$d" .
		for f in "${FLAGS[@]}"; do
			check "$CORPUS/$d" "$f" .
		done
	done
	for f in "${FLAGS[@]}"; do
		check "$CORPUS/masse" "$f" .
		check "$CORPUS/droits" "$f" ok
	done
	end_cat
}

cat_paires()
{
	skip_cat "paires d options" && return
	start_cat "paires d options"
	local i j n=${#FLAGS[@]}
	for ((i = 0; i < n; i++)); do
		for ((j = i + 1; j < n; j++)); do
			check "$CORPUS/types" "${FLAGS[i]}" "${FLAGS[j]}" .
			check "$CORPUS/ext" "${FLAGS[i]}" "${FLAGS[j]}" .
			check "$CORPUS/noms" "${FLAGS[i]}" "${FLAGS[j]}" .
		done
	done
	end_cat
}

cat_combinaisons()
{
	skip_cat "combinaisons tirees au sort" && return
	start_cat "combinaisons tirees au sort"
	local i a b c e n=${#FLAGS[@]}
	RANDOM=20260829
	for ((i = 0; i < 400; i++)); do
		a=${FLAGS[RANDOM % n]}; b=${FLAGS[RANDOM % n]}; c=${FLAGS[RANDOM % n]}
		check "$CORPUS/types" "$a" "$b" "$c" .
	done
	for ((i = 0; i < 300; i++)); do
		a=${FLAGS[RANDOM % n]}; b=${FLAGS[RANDOM % n]}
		c=${FLAGS[RANDOM % n]}; e=${FLAGS[RANDOM % n]}
		check "$CORPUS/arbre" "$a" "$b" "$c" "$e" .
	done
	end_cat
}

cat_format_long()
{
	skip_cat "format long" && return
	start_cat "format long"
	local d o
	for d in types ext tri noms droits acl arbre; do
		for o in -l -la -lA -li -ls -lis -ln -lg -lo -lgo -lh -lhs -lG -lF \
			-lp -ld -lin -lS -lt -lc -lu -lU -lr; do
			check "$CORPUS/$d" "$o" .
		done
		check "$CORPUS/$d" --format=long .
		check "$CORPUS/$d" --format=verbose .
	done
	check "$CORPUS/types" -l lien lien_dossier lien_casse boucle_a soi_meme
	check "$CORPUS/types" -l /dev/null
	check "$CORPUS/types" -l /dev/null /dev/zero
	check "$CORPUS/types" -li /dev/null
	check "$CORPUS/types" -ls /dev/null
	end_cat
}

cat_tris()
{
	skip_cat "tris" && return
	start_cat "tris"
	local d s t
	for d in tri types ext noms masse; do
		for s in none size time extension name; do
			check "$CORPUS/$d" "--sort=$s" .
			check "$CORPUS/$d" "--sort=$s" -r .
			check "$CORPUS/$d" "--sort=$s" -l .
			check "$CORPUS/$d" "--sort=$s" -lr .
		done
		for t in atime access use ctime status mtime modification; do
			check "$CORPUS/$d" "--time=$t" -l .
			check "$CORPUS/$d" "--time=$t" -lt .
			check "$CORPUS/$d" "--time=$t" -ltr .
		done
		for s in -t -tr -S -Sr -U -X -Xr -f -fl -ltu -ltc -rt -rS; do
			check "$CORPUS/$d" "$s" .
		done
	done
	end_cat
}

cat_colonnes()
{
	skip_cat "colonnes et largeurs" && return
	start_cat "colonnes et largeurs"
	local w d
	for d in court larg long ext noms; do
		for w in 1 2 3 4 5 6 7 8 9 10 12 14 16 20 24 30 40 50 60 79 80 81 100 132 200; do
			check "$CORPUS/$d" -C -w "$w" .
			check "$CORPUS/$d" -x -w "$w" .
			check "$CORPUS/$d" -1 -w "$w" .
			check "$CORPUS/$d" -C -w "$w" -F .
			check "$CORPUS/$d" -C -w "$w" -i .
		done
	done
	for w in $(seq 1 80); do
		check "$CORPUS/masse" -C -w "$w" .
		check "$CORPUS/larg" -x -w "$w" .
		check "$CORPUS/court" -C -w "$w" .
	done
	for w in 20 40 80 120; do
		check_env "COLUMNS=$w" "$CORPUS/court" -C .
		check_env "COLUMNS=$w" "$CORPUS/larg" -x .
		check_env "COLUMNS=$w" "$CORPUS/court" .
	done
	check "$CORPUS/court" -w 0 .
	check "$CORPUS/court" --width=45 .
	check "$CORPUS/court" -w 18446744073709551615 .
	end_cat
}

cat_couleurs()
{
	skip_cat "couleurs" && return
	start_cat "couleurs"
	local c d
	local -a JEUX=(
		""
		"di=01;34:ln=01;36:ex=01;32"
		"di=01;34:ln=target:ex=01;32"
		"or=05;37;41:mi=01;05;37;41:ln=01;36"
		"*.c=01;33:*.h=00;33:*.tar=01;31:*.gz=01;31"
		"su=37;41:sg=30;43:tw=30;42:ow=34;42:st=37;44"
		"pi=40;33:so=01;35:bd=40;33;01:cd=40;33;01"
		"rs=0:di=:ln="
		"di=01;34:"
		"fi=00;37:di=07;34"
	)
	for c in "${JEUX[@]}"; do
		for d in types ext noms; do
			check_env "LS_COLORS=$c" "$CORPUS/$d" --color=always .
			check_env "LS_COLORS=$c" "$CORPUS/$d" --color=always -l .
			check_env "LS_COLORS=$c" "$CORPUS/$d" --color=always -F .
			check_env "LS_COLORS=$c" "$CORPUS/$d" --color=always -a .
			check_env "LS_COLORS=$c" "$CORPUS/$d" --color=always -R .
			check_env "LS_COLORS=$c" "$CORPUS/$d" --color=always -C -w 40 .
		done
	done
	for c in always yes force never no none auto tty if-tty; do
		check "$CORPUS/types" "--color=$c" .
		check "$CORPUS/types" "--color=$c" -l .
	done
	check "$CORPUS/types" --color .
	check "$CORPUS/types" -G .
	check_env "LS_COLORS=di=01;34" "$CORPUS/droits" --color=always -l .
	check_env "LS_COLORS=or=31:mi=41" "$CORPUS/types" --color=always -l .
	for c in "TERM=" "TERM=dumb" "TERM=zorglub" "TERM=xterm" "TERM=xterm-256color" \
		"TERM=screen" "TERM=st-256color" "TERM=linux" "TERM=vt220" "TERM=Eterm" \
		"TERM=con80x25" "TERM=xterm-direct" "TERM=rxvt-unicode" \
		"TERM=dumb COLORTERM=truecolor" "TERM=dumb COLORTERM=" \
		"TERM=dumb LS_COLORS=di=01;33" "TERM=zorglub LS_COLORS=" ; do
		check_env "$c" "$CORPUS/types" --color=always .
		check_env "$c" "$CORPUS/types" --color=always -l .
		check_env "$c" "$CORPUS/ext" --color=always -F .
		check_env "$c" "$CORPUS/types" -G .
	done
	end_cat
}

cat_indicateurs()
{
	skip_cat "indicateurs -F -p" && return
	start_cat "indicateurs -F -p"
	local s d
	for d in types ext arbre noms; do
		check "$CORPUS/$d" -F .
		check "$CORPUS/$d" -p .
		check "$CORPUS/$d" -Fa .
		check "$CORPUS/$d" -lF .
		check "$CORPUS/$d" -FR .
		check "$CORPUS/$d" -Fx .
		check "$CORPUS/$d" -F -w 30 .
		for s in none slash file-type classify; do
			check "$CORPUS/$d" "--indicator-style=$s" .
			check "$CORPUS/$d" "--indicator-style=$s" -l .
		done
		check "$CORPUS/$d" --classify .
		check "$CORPUS/$d" --classify=always .
		check "$CORPUS/$d" --classify=never .
		check "$CORPUS/$d" --classify=auto .
	done
	end_cat
}

cat_recursion()
{
	skip_cat "recursion" && return
	start_cat "recursion"
	local d o
	for d in arbre types droits acl noms; do
		for o in -R -Ra -RA -lR -lRa -Rt -RU -Rr -Rd -RF -R1 -Rx -RC -lRt; do
			check "$CORPUS/$d" "$o" .
		done
	done
	check "$CORPUS" -R arbre
	check "$CORPUS" -R arbre/a
	check "$CORPUS" -R arbre/a/b/c
	check "$CORPUS" -lR arbre arbre/a
	check "$CORPUS" -R rien
	check "$CORPUS" -R arbre types
	check "$CORPUS/arbre" -R z
	end_cat
}

cat_operandes()
{
	skip_cat "operandes et chemins" && return
	start_cat "operandes et chemins"
	check "$CORPUS" types
	check "$CORPUS" types ext
	check "$CORPUS" -l types ext
	check "$CORPUS" types/petit
	check "$CORPUS" types/petit types/vide
	check "$CORPUS" -l types/petit types
	check "$CORPUS" types/petit types ext
	check "$CORPUS" -d types
	check "$CORPUS" -d types ext
	check "$CORPUS" -d .
	check "$CORPUS" -ld .
	check "$CORPUS" .
	check "$CORPUS" ..
	check "$CORPUS" -l ..
	check "$CORPUS" /
	check "$CORPUS" -d /
	check "$CORPUS" -ld /
	check "$CORPUS" -d //
	check "$CORPUS" -ld //
	check "$CORPUS" -d ///
	check "$CORPUS" types/
	check "$CORPUS" types//
	check "$CORPUS" types///
	check "$CORPUS" -R types/
	check "$CORPUS" -l types/
	check "$CORPUS" arbre/a/
	check "$CORPUS" -R arbre/a//
	check "$CORPUS" ./types
	check "$CORPUS" ././types
	check "$CORPUS" -l /dev/null
	check "$CORPUS" -l /etc/hosts
	check "$CORPUS/noms" -- -tiret
	check "$CORPUS/noms" -l -- --double-tiret
	check "$CORPUS/noms" -l "nom avec espaces"
	check "$CORPUS/noms" -l "apostrophe's"
	check "$CORPUS" -l types ext tri noms arbre rien
	check "$CORPUS" types ext types
	check "$CORPUS" -l types/petit types/petit
	check "$CORPUS" -lt types ext tri
	check "$CORPUS" -ld types ext tri
	check "$CORPUS" -lR types/dossier
	check "$CORPUS/types" -l lien
	check "$CORPUS/types" -ld lien
	check "$CORPUS/types" -ld lien_dossier
	check "$CORPUS/types" -l lien_dossier
	check "$CORPUS/types" lien_dossier
	check "$CORPUS/types" -l lien_casse
	end_cat
}

cat_erreurs()
{
	skip_cat "erreurs et codes retour" && return
	start_cat "erreurs et codes retour"
	check "$CORPUS" nexistepas
	check "$CORPUS" -l nexistepas
	check "$CORPUS" nexistepas types
	check "$CORPUS" types nexistepas
	check "$CORPUS" nexistepas encore_moins
	check "$CORPUS" -R nexistepas
	check "$CORPUS" types/petit/impossible
	check "$CORPUS" -l types/petit/impossible
	check "$CORPUS" ""
	check "$CORPUS" -l ""
	if [ "$IS_ROOT" = "0" ]; then
		check "$CORPUS/droits" -l .
		check "$CORPUS/droits" -lR .
		check "$CORPUS/droits" interdit
		check "$CORPUS/droits" -l interdit
		check "$CORPUS/droits" -R interdit
		check "$CORPUS/droits" -l lisible_non_traversable
		check "$CORPUS/droits" -li lisible_non_traversable
		check "$CORPUS/droits" -ls lisible_non_traversable
		check "$CORPUS/droits" -lt lisible_non_traversable
		check "$CORPUS/droits" -lS lisible_non_traversable
		check "$CORPUS/droits" -lF lisible_non_traversable
		check "$CORPUS/droits" -lh lisible_non_traversable
		check "$CORPUS/droits" --color=always -l lisible_non_traversable
		check "$CORPUS/droits" lisible_non_traversable
		check "$CORPUS/droits" -R lisible_non_traversable
		check "$CORPUS/droits" -l interdit ok nexistepas
		check "$CORPUS/droits" -lR interdit ok
	fi
	end_cat
}

cat_options_invalides()
{
	skip_cat "options invalides" && return
	start_cat "options invalides"
	local o
	for o in -e -E -j -J -K -M -O -P -V -W -y --bogus --nope --sortt; do
		check "$CORPUS/types" "$o" .
	done
	check "$CORPUS/types" -w
	check "$CORPUS/types" -w abc .
	check "$CORPUS/types" -w -3 .
	check "$CORPUS/types" -w 40abc .
	check "$CORPUS/types" --width=abc .
	check "$CORPUS/types" --color=bogus .
	check "$CORPUS/types" --indicator-style=bogus .
	check "$CORPUS/types" --classify=bogus .
	end_cat
}

cat_options_longues()
{
	skip_cat "options longues" && return
	start_cat "options longues"
	local o
	for o in --all --almost-all --directory --human-readable --inode \
		--numeric-uid-gid --no-group --reverse --recursive --size; do
		check "$CORPUS/types" "$o" .
		check "$CORPUS/types" "$o" -l .
	done
	check "$CORPUS/types" --all --recursive
	check "$CORPUS/types" --reverse --size --inode -l .
	check "$CORPUS/types" --rec .
	check "$CORPUS/types" --hum -l .
	check "$CORPUS/types" --no-g -l .
	check "$CORPUS/types" --sort=none --reverse .
	end_cat
}

cat_acl()
{
	skip_cat "acl" && return
	if [ "$HAVE_ACL" = "0" ]; then
		printf '%s%-32s%s        skip (pas de support acl ici)\n' \
			"$YELLOW" "acl" "$RESET"
		return
	fi
	start_cat "acl"
	local o
	for o in -l -la -lR -li -ls -ln -lh -lg -lo -lt -lS -lF -ld -lU -lc -lu; do
		check "$CORPUS/acl" "$o" .
	done
	check "$CORPUS/acl" --color=always -l .
	check "$CORPUS/acl" -l f2
	check "$CORPUS/acl" -l f1
	check "$CORPUS/acl" -l f1 f2
	check "$CORPUS/acl" -l f2 f1
	check "$CORPUS/acl" -l f2 f5
	check "$CORPUS/acl" -l f1 f2 f3 f4 f5 f6
	check "$CORPUS/acl" -l d1
	check "$CORPUS/acl" -ld d1
	check "$CORPUS/acl" -ld d1 d2
	check "$CORPUS/acl" -ld d2 d1
	check "$CORPUS/acl" .
	check "$CORPUS/acl" -F .
	check "$CORPUS/acl" -R .
	end_cat
}

cat_formats()
{
	skip_cat "formats" && return
	start_cat "formats"
	local f d
	for d in types court larg noms masse; do
		for f in across horizontal vertical single-column long verbose; do
			check "$CORPUS/$d" "--format=$f" .
			check "$CORPUS/$d" "--format=$f" -w 40 .
			check "$CORPUS/$d" "--format=$f" -w 20 .
		done
		for f in -1 -C -x -1C -Cx -xC -l1 -1l -lC -Cl; do
			check "$CORPUS/$d" "$f" .
		done
	done
	end_cat
}

cat_divers()
{
	skip_cat "divers" && return
	start_cat "divers"
	local o
	for o in -a -A -s -sh -i -is -h -1 -x -lS -R -lh -lhS -sh -li -F; do
		check "$CORPUS/types" "$o" .
		check "$CORPUS/tri" "$o" .
		check "$CORPUS/rien" "$o" .
	done
	check "$CORPUS/types" -la .. .
	check "$CORPUS/masse" -l .
	check "$CORPUS/rien" -la .
	check "$CORPUS/rien" -lR .
	check_env "TZ=Europe/Paris" "$CORPUS/tri" -l .
	check_env "TZ=Pacific/Kiritimati" "$CORPUS/tri" -lt .
	check_env "TZ=UTC" "$CORPUS/tri" -l --time=ctime .
	end_cat
}

printf '%sft_ls%s   %s\n' "$BOLD" "$RESET" "$FT"
printf '%sGNU%s     %s (%s)\n' "$BOLD" "$RESET" "$GNU" \
	"$("$GNU" --version | head -1)"
printf '%scorpus%s  %s\n\n' "$BOLD" "$RESET" "$CORPUS"

build_corpus

cat_options_simples
cat_format_long
cat_formats
cat_tris
cat_colonnes
cat_couleurs
cat_indicateurs
cat_recursion
cat_operandes
cat_erreurs
cat_options_invalides
cat_options_longues
cat_acl
cat_divers
cat_paires
cat_combinaisons

echo
printf '%s%s%s\n' "$BOLD" "-----------------------------------------------------------" "$RESET"
for line in "${REPORT[@]}"; do printf '%s\n' "$line"; done
printf '%s%s%s\n' "$BOLD" "-----------------------------------------------------------" "$RESET"
if [ "$FAILED" = "0" ]; then
	printf '  %s%sTOTAL %d cas, 0 ecart%s\n' "$BOLD" "$GREEN" "$TOTAL" "$RESET"
	exit 0
fi
printf '  %s%sTOTAL %d cas, %d ecarts%s\n' "$BOLD" "$RED" "$TOTAL" "$FAILED" "$RESET"
exit 1
