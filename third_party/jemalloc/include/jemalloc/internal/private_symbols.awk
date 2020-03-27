#!/usr/bin/env awk -f

BEGIN {
  sym_prefix = "_"
  split("\
        _aligned_alloc \
        _calloc \
        _dallocx \
        _free \
        _mallctl \
        _mallctlbymib \
        _mallctlnametomib \
        _malloc \
        _malloc_conf \
        _malloc_message \
        _malloc_stats_print \
        _malloc_usable_size \
        _mallocx \
        _smallocx_4a78c6d81b3f431070f362c29ab7b492ee0b9e70 \
        _nallocx \
        _posix_memalign \
        _rallocx \
        _realloc \
        _sallocx \
        _sdallocx \
        _xallocx \
        _valloc \
        _pthread_create \
        ", exported_symbol_names)
  # Store exported symbol names as keys in exported_symbols.
  for (i in exported_symbol_names) {
    exported_symbols[exported_symbol_names[i]] = 1
  }
}

# Process 'nm -a <c_source.o>' output.
#
# Handle lines like:
#   0000000000000008 D opt_junk
#   0000000000007574 T malloc_initialized
(NF == 3 && $2 ~ /^[ABCDGRSTVW]$/ && !($3 in exported_symbols) && $3 ~ /^[A-Za-z0-9_]+$/) {
  print substr($3, 1+length(sym_prefix), length($3)-length(sym_prefix))
}

# Process 'dumpbin /SYMBOLS <c_source.obj>' output.
#
# Handle lines like:
#   353 00008098 SECT4  notype       External     | opt_junk
#   3F1 00000000 SECT7  notype ()    External     | malloc_initialized
($3 ~ /^SECT[0-9]+/ && $(NF-2) == "External" && !($NF in exported_symbols)) {
  print $NF
}
