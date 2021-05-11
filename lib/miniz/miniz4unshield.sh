#!/bin/sh

# commit: 9edb278d22ad5545fb4561595a4d96eaab55ffa3

if ! [ -f amalgamate.sh ] ; then
	echo "amalgamate.sh is missing"
	echo "run this inside the miniz git repo"
	exit 1
fi
if ! [ -d .git ] ; then
	echo "need a git repo"
	exit 1
fi

#cat miniz.h miniz_common.h miniz_tinfl.h > miniz_zzz.h
#cat miniz.c miniz_tinfl.c > miniz_zzz.c

git checkout -- .

rm -rf _build

#echo -n > miniz_tdef.c
#echo -n > miniz_tdef.h

cat > miniz_tdef.c <<EOF
mz_uint tdefl_create_comp_flags_from_zip_params(int level, int window_bits, int strategy)
{
    return 0;
}

tdefl_status tdefl_init(tdefl_compressor *d, tdefl_put_buf_func_ptr pPut_buf_func, void *pPut_buf_user, int flags)
{
    return TDEFL_STATUS_OKAY;
}

tdefl_status tdefl_compress(tdefl_compressor *d, const void *pIn_buf, size_t *pIn_buf_size, void *pOut_buf, size_t *pOut_buf_size, tdefl_flush flush)
{
    return TDEFL_STATUS_OKAY;
}

mz_uint32 tdefl_get_adler32(tdefl_compressor *d)
{
    return d->m_adler32;
}
EOF

echo -n > miniz_zip.c
echo -n > miniz_zip.h

./amalgamate.sh
