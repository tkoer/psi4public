set(SRC 
    G.cc
    G_norm.cc
    Gabcd.cc
    Gciab.cc
    Gibja.cc
    Gijab.cc
    Gijab_RHF.cc
    Gijab_ROHF.cc
    Gijab_UHF.cc
    Gijka.cc
    Gijkl.cc
    Iab.cc
    Iai.cc
    Iia.cc
    Iij.cc
    V.cc
    add_core_ROHF.cc
    add_core_UHF.cc
    add_ref.cc
    add_ref_RHF.cc
    add_ref_ROHF.cc
    add_ref_UHF.cc
    ael.cc
    build_A.cc
    build_A_RHF.cc
    build_A_ROHF.cc
    build_A_UHF.cc
    build_X.cc
    build_Z.cc
    build_Z_RHF.cc
    build_Z_ROHF.cc
    build_Z_UHF.cc
    build_ex_tdensity.cc
    cache.cc
    ccdensity.cc
    classify.cc
    deanti.cc
    deanti_RHF.cc
    deanti_ROHF.cc
    deanti_UHF.cc
    densgrid_RHF.cc
    dipole.cc
    distribute.cc
    dump_RHF.cc
    dump_ROHF.cc
    dump_UHF.cc
    energy.cc
    energy_RHF.cc
    energy_ROHF.cc
    energy_UHF.cc
    ex_oscillator_strength.cc
    ex_rotational_strength.cc
    ex_sort_td_rohf.cc
    ex_sort_td_uhf.cc
    ex_td_cleanup.cc
    ex_td_print.cc
    ex_td_setup.cc
    ex_tdensity.cc
    ex_tdensity_intermediates.cc
    file_build.cc
    fold.cc
    fold_RHF.cc
    fold_ROHF.cc
    fold_UHF.cc
    get_frozen.cc
    get_moinfo.cc
    get_params.cc
    get_rho_params.cc
    get_td_params.cc
    idx_error.cc
    idx_permute.cc
    kinetic.cc
    lag.cc
    ltdensity.cc
    ltdensity_intermediates.cc
    norm.cc
    onepdm.cc
    oscillator_strength.cc
    relax_D.cc
    relax_I.cc
    relax_I_RHF.cc
    relax_I_ROHF.cc
    relax_I_UHF.cc
    resort_gamma.cc
    resort_tei.cc
    rotational_strength.cc
    rtdensity.cc
    setup_LR.cc
    sortI.cc
    sortI_RHF.cc
    sortI_ROHF.cc
    sortI_UHF.cc
    sort_ltd_rohf.cc
    sort_ltd_uhf.cc
    sort_rtd_rohf.cc
    sort_rtd_uhf.cc
    sortone.cc
    sortone_RHF.cc
    sortone_ROHF.cc
    sortone_UHF.cc
    td_cleanup.cc
    td_print.cc
    td_setup.cc
    tdensity.cc
    transL.cc
    transdip.cc
    transp.cc
    twopdm.cc
    x_Gabcd.cc
    x_Gciab.cc
    x_Gciab_uhf.cc
    x_Gibja.cc
    x_Gibja_uhf.cc
    x_Gijab.cc
    x_Gijab_uhf.cc
    x_Gijka.cc
    x_Gijka_uhf.cc
    x_Gijkl.cc
    x_V.cc
    x_oe_intermediates.cc
    x_oe_intermediates_rhf.cc
    x_onepdm.cc
    x_onepdm_uhf.cc
    x_te_intermediates.cc
    x_te_intermediates_rhf.cc
    x_xi1.cc
    x_xi1_connected.cc
    x_xi1_rhf.cc
    x_xi1_uhf.cc
    x_xi2.cc
    x_xi2_rhf.cc
    x_xi2_uhf.cc
    x_xi_check.cc
    x_xi_intermediates.cc
    zero_pdm.cc
)
add_library(ccdensity ${SRC})
add_dependencies(ccdensity mints)
