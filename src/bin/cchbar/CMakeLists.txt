set(SRC cache.cc cc2_Wabei.cc cc2_Wmbej.cc cc2_Wmbij.cc cc2_Zmbej.cc cc3_HET1.cc cchbar.cc F.cc Fai.cc get_moinfo.cc get_params.cc HET1_Wabef.cc norm_HET1.cc purge.cc reference.cc sort_amps.cc status.cc tau.cc taut.cc Wabei.cc Wabei_AAAA_UHF.cc Wabei_ABAB_UHF.cc Wabei_BABA_UHF.cc Wabei_BBBB_UHF.cc Wabei_RHF.cc Wabei_RHF_FT2_a.cc Wabei_ROHF.cc Wabij.cc Wamef.cc Wmbej.cc Wmbij.cc Wmnie.cc)
add_library(cchbar ${SRC})
add_dependencies(cchbar mints)
