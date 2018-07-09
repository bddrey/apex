#
# Rule to build imxrt xip image
#
define fn_imxrt_xip_rule
    # fn_imxrt_xip_rule

    # include built object flags
    -include $(tgt).xipflags

    # set target variables
    $(tgt)_LDSCRIPT := $$(call fn_path_join,$$(mk_dir),$$(LDSCRIPT))

    $$(eval $$(call fn_process_sources))
    $$(eval $$(call fn_flags_rule,$(tgt).xipflags,$$($(tgt)_OBJS) $$(tgt)_LDSCRIPT))

    .INTERMEDIATE: $(tgt).o
    $(tgt).o: $$($(tgt)_OBJS) $$($(tgt)_LDSCRIPT) $(tgt).xipflags
	rm -f $$@
	$$($(tgt)_LD) -o $$@ -T $$($(tgt)_LDSCRIPT) $$($(tgt)_OBJS)

    .INTERMEDIATE: $(tgt).bin
    $(tgt).bin: $(tgt).o
	$$($(tgt)_OBJCOPY) -O binary $$< $$@

    $(tgt): $(tgt).bin $(IMG)
	cat $$^ > $$@
endef
