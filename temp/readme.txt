mtk_agpsd: binary-patch to ICU 56 compatibility
... by simply doing this:

```sh
sed -i 's/\([Uu][Cc][Nn][Vv]_[A-Za-z_]*\)_55/\1_56/g' mtk_agpsd
```

Change-Id: I492d92448b5ef2a7505e93bfe4ed2661f5723516