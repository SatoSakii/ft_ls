# tests

`ft_ls` et le vrai `ls` de GNU sont lances cote a cote sur les memes arguments,
et doivent rendre exactement la meme chose.

```console
$ make test
```

## Un cas

Les deux binaires tournent dans le meme dossier, avec `LC_ALL=C`, `TZ=UTC`, et
`LS_COLORS` / `COLUMNS` / `BLOCK_SIZE` / `TIME_STYLE` / `QUOTING_STYLE` vides.

Un echec affiche le `diff` passe par `cat -v`, GNU a gauche, ft_ls a droite.