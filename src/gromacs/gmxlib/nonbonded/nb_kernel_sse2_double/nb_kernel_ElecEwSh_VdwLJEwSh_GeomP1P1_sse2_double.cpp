/*
 * This file is part of the GROMACS molecular simulation package.
 *
 * Copyright (c) 2012,2013,2014,2015,2017,2018, by the GROMACS development team, led by
 * Mark Abraham, David van der Spoel, Berk Hess, and Erik Lindahl,
 * and including many others, as listed in the AUTHORS file in the
 * top-level source directory and at http://www.gromacs.org.
 *
 * GROMACS is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 *
 * GROMACS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with GROMACS; if not, see
 * http://www.gnu.org/licenses, or write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA.
 *
 * If you want to redistribute modifications to GROMACS, please
 * consider that scientific software is very special. Version
 * control is crucial - bugs must be traceable. We will be happy to
 * consider code for inclusion in the official distribution, but
 * derived work must not be called official GROMACS. Details are found
 * in the README & COPYING files - if they are missing, get the
 * official version at http://www.gromacs.org.
 *
 * To help us fund GROMACS development, we humbly ask that you cite
 * the research papers on the package. Check out http://www.gromacs.org.
 */
/*
 * Note: this file was generated by the GROMACS sse2_double kernel generator.
 */
#include "gmxpre.h"

#include "config.h"

#include <math.h>

#include "../nb_kernel.h"
#include "gromacs/gmxlib/nrnb.h"

#include "kernelutil_x86_sse2_double.h"

/*
 * Gromacs nonbonded kernel:   nb_kernel_ElecEwSh_VdwLJEwSh_GeomP1P1_VF_sse2_double
 * Electrostatics interaction: Ewald
 * VdW interaction:            LJEwald
 * Geometry:                   Particle-Particle
 * Calculate force/pot:        PotentialAndForce
 */
void
nb_kernel_ElecEwSh_VdwLJEwSh_GeomP1P1_VF_sse2_double
                    (t_nblist                    * gmx_restrict       nlist,
                     rvec                        * gmx_restrict          xx,
                     rvec                        * gmx_restrict          ff,
                     struct t_forcerec           * gmx_restrict          fr,
                     t_mdatoms                   * gmx_restrict     mdatoms,
                     nb_kernel_data_t gmx_unused * gmx_restrict kernel_data,
                     t_nrnb                      * gmx_restrict        nrnb)
{
    /* Suffixes 0,1,2,3 refer to particle indices for waters in the inner or outer loop, or
     * just 0 for non-waters.
     * Suffixes A,B refer to j loop unrolling done with SSE double precision, e.g. for the two different
     * jnr indices corresponding to data put in the four positions in the SIMD register.
     */
    int              i_shift_offset,i_coord_offset,outeriter,inneriter;
    int              j_index_start,j_index_end,jidx,nri,inr,ggid,iidx;
    int              jnrA,jnrB;
    int              j_coord_offsetA,j_coord_offsetB;
    int              *iinr,*jindex,*jjnr,*shiftidx,*gid;
    real             rcutoff_scalar;
    real             *shiftvec,*fshift,*x,*f;
    __m128d          tx,ty,tz,fscal,rcutoff,rcutoff2,jidxall;
    int              vdwioffset0;
    __m128d          ix0,iy0,iz0,fix0,fiy0,fiz0,iq0,isai0;
    int              vdwjidx0A,vdwjidx0B;
    __m128d          jx0,jy0,jz0,fjx0,fjy0,fjz0,jq0,isaj0;
    __m128d          dx00,dy00,dz00,rsq00,rinv00,rinvsq00,r00,qq00,c6_00,c12_00;
    __m128d          velec,felec,velecsum,facel,crf,krf,krf2;
    real             *charge;
    int              nvdwtype;
    __m128d          rinvsix,rvdw,vvdw,vvdw6,vvdw12,fvdw,fvdw6,fvdw12,vvdwsum,sh_vdw_invrcut6;
    int              *vdwtype;
    real             *vdwparam;
    __m128d          one_sixth   = _mm_set1_pd(1.0/6.0);
    __m128d          one_twelfth = _mm_set1_pd(1.0/12.0);
    __m128d           c6grid_00;
    __m128d           ewclj,ewclj2,ewclj6,ewcljrsq,poly,exponent,f6A,f6B,sh_lj_ewald;
    real             *vdwgridparam;
    __m128d           one_half = _mm_set1_pd(0.5);
    __m128d           minus_one = _mm_set1_pd(-1.0);
    __m128i          ewitab;
    __m128d          ewtabscale,eweps,sh_ewald,ewrt,ewtabhalfspace,ewtabF,ewtabFn,ewtabD,ewtabV;
    real             *ewtab;
    __m128d          dummy_mask,cutoff_mask;
    __m128d          signbit   = gmx_mm_castsi128_pd( _mm_set_epi32(0x80000000,0x00000000,0x80000000,0x00000000) );
    __m128d          one     = _mm_set1_pd(1.0);
    __m128d          two     = _mm_set1_pd(2.0);
    x                = xx[0];
    f                = ff[0];

    nri              = nlist->nri;
    iinr             = nlist->iinr;
    jindex           = nlist->jindex;
    jjnr             = nlist->jjnr;
    shiftidx         = nlist->shift;
    gid              = nlist->gid;
    shiftvec         = fr->shift_vec[0];
    fshift           = fr->fshift[0];
    facel            = _mm_set1_pd(fr->ic->epsfac);
    charge           = mdatoms->chargeA;
    nvdwtype         = fr->ntype;
    vdwparam         = fr->nbfp;
    vdwtype          = mdatoms->typeA;
    vdwgridparam     = fr->ljpme_c6grid;
    sh_lj_ewald      = _mm_set1_pd(fr->ic->sh_lj_ewald);
    ewclj            = _mm_set1_pd(fr->ic->ewaldcoeff_lj);
    ewclj2           = _mm_mul_pd(minus_one,_mm_mul_pd(ewclj,ewclj));

    sh_ewald         = _mm_set1_pd(fr->ic->sh_ewald);
    ewtab            = fr->ic->tabq_coul_FDV0;
    ewtabscale       = _mm_set1_pd(fr->ic->tabq_scale);
    ewtabhalfspace   = _mm_set1_pd(0.5/fr->ic->tabq_scale);

    /* When we use explicit cutoffs the value must be identical for elec and VdW, so use elec as an arbitrary choice */
    rcutoff_scalar   = fr->ic->rcoulomb;
    rcutoff          = _mm_set1_pd(rcutoff_scalar);
    rcutoff2         = _mm_mul_pd(rcutoff,rcutoff);

    sh_vdw_invrcut6  = _mm_set1_pd(fr->ic->sh_invrc6);
    rvdw             = _mm_set1_pd(fr->ic->rvdw);

    /* Avoid stupid compiler warnings */
    jnrA = jnrB = 0;
    j_coord_offsetA = 0;
    j_coord_offsetB = 0;

    outeriter        = 0;
    inneriter        = 0;

    /* Start outer loop over neighborlists */
    for(iidx=0; iidx<nri; iidx++)
    {
        /* Load shift vector for this list */
        i_shift_offset   = DIM*shiftidx[iidx];

        /* Load limits for loop over neighbors */
        j_index_start    = jindex[iidx];
        j_index_end      = jindex[iidx+1];

        /* Get outer coordinate index */
        inr              = iinr[iidx];
        i_coord_offset   = DIM*inr;

        /* Load i particle coords and add shift vector */
        gmx_mm_load_shift_and_1rvec_broadcast_pd(shiftvec+i_shift_offset,x+i_coord_offset,&ix0,&iy0,&iz0);

        fix0             = _mm_setzero_pd();
        fiy0             = _mm_setzero_pd();
        fiz0             = _mm_setzero_pd();

        /* Load parameters for i particles */
        iq0              = _mm_mul_pd(facel,_mm_load1_pd(charge+inr+0));
        vdwioffset0      = 2*nvdwtype*vdwtype[inr+0];

        /* Reset potential sums */
        velecsum         = _mm_setzero_pd();
        vvdwsum          = _mm_setzero_pd();

        /* Start inner kernel loop */
        for(jidx=j_index_start; jidx<j_index_end-1; jidx+=2)
        {

            /* Get j neighbor index, and coordinate index */
            jnrA             = jjnr[jidx];
            jnrB             = jjnr[jidx+1];
            j_coord_offsetA  = DIM*jnrA;
            j_coord_offsetB  = DIM*jnrB;

            /* load j atom coordinates */
            gmx_mm_load_1rvec_2ptr_swizzle_pd(x+j_coord_offsetA,x+j_coord_offsetB,
                                              &jx0,&jy0,&jz0);

            /* Calculate displacement vector */
            dx00             = _mm_sub_pd(ix0,jx0);
            dy00             = _mm_sub_pd(iy0,jy0);
            dz00             = _mm_sub_pd(iz0,jz0);

            /* Calculate squared distance and things based on it */
            rsq00            = gmx_mm_calc_rsq_pd(dx00,dy00,dz00);

            rinv00           = sse2_invsqrt_d(rsq00);

            rinvsq00         = _mm_mul_pd(rinv00,rinv00);

            /* Load parameters for j particles */
            jq0              = gmx_mm_load_2real_swizzle_pd(charge+jnrA+0,charge+jnrB+0);
            vdwjidx0A        = 2*vdwtype[jnrA+0];
            vdwjidx0B        = 2*vdwtype[jnrB+0];

            /**************************
             * CALCULATE INTERACTIONS *
             **************************/

            if (gmx_mm_any_lt(rsq00,rcutoff2))
            {

            r00              = _mm_mul_pd(rsq00,rinv00);

            /* Compute parameters for interactions between i and j atoms */
            qq00             = _mm_mul_pd(iq0,jq0);
            gmx_mm_load_2pair_swizzle_pd(vdwparam+vdwioffset0+vdwjidx0A,
                                         vdwparam+vdwioffset0+vdwjidx0B,&c6_00,&c12_00);

            c6grid_00       = gmx_mm_load_2real_swizzle_pd(vdwgridparam+vdwioffset0+vdwjidx0A,
                                                               vdwgridparam+vdwioffset0+vdwjidx0B);

            /* EWALD ELECTROSTATICS */

            /* Calculate Ewald table index by multiplying r with scale and truncate to integer */
            ewrt             = _mm_mul_pd(r00,ewtabscale);
            ewitab           = _mm_cvttpd_epi32(ewrt);
            eweps            = _mm_sub_pd(ewrt,_mm_cvtepi32_pd(ewitab));
            ewitab           = _mm_slli_epi32(ewitab,2);
            ewtabF           = _mm_load_pd( ewtab + gmx_mm_extract_epi32(ewitab,0) );
            ewtabD           = _mm_load_pd( ewtab + gmx_mm_extract_epi32(ewitab,1) );
            GMX_MM_TRANSPOSE2_PD(ewtabF,ewtabD);
            ewtabV           = _mm_load_sd( ewtab + gmx_mm_extract_epi32(ewitab,0) +2);
            ewtabFn          = _mm_load_sd( ewtab + gmx_mm_extract_epi32(ewitab,1) +2);
            GMX_MM_TRANSPOSE2_PD(ewtabV,ewtabFn);
            felec            = _mm_add_pd(ewtabF,_mm_mul_pd(eweps,ewtabD));
            velec            = _mm_sub_pd(ewtabV,_mm_mul_pd(_mm_mul_pd(ewtabhalfspace,eweps),_mm_add_pd(ewtabF,felec)));
            velec            = _mm_mul_pd(qq00,_mm_sub_pd(_mm_sub_pd(rinv00,sh_ewald),velec));
            felec            = _mm_mul_pd(_mm_mul_pd(qq00,rinv00),_mm_sub_pd(rinvsq00,felec));

            /* Analytical LJ-PME */
            rinvsix          = _mm_mul_pd(_mm_mul_pd(rinvsq00,rinvsq00),rinvsq00);
            ewcljrsq         = _mm_mul_pd(ewclj2,rsq00);
            ewclj6           = _mm_mul_pd(ewclj2,_mm_mul_pd(ewclj2,ewclj2));
            exponent         = sse2_exp_d(ewcljrsq);
            /* poly = exp(-(beta*r)^2) * (1 + (beta*r)^2 + (beta*r)^4 /2) */
            poly             = _mm_mul_pd(exponent,_mm_add_pd(_mm_sub_pd(one,ewcljrsq),_mm_mul_pd(_mm_mul_pd(ewcljrsq,ewcljrsq),one_half)));
            /* vvdw6 = [C6 - C6grid * (1-poly)]/r6 */
            vvdw6            = _mm_mul_pd(_mm_sub_pd(c6_00,_mm_mul_pd(c6grid_00,_mm_sub_pd(one,poly))),rinvsix);
            vvdw12           = _mm_mul_pd(c12_00,_mm_mul_pd(rinvsix,rinvsix));
            vvdw             = _mm_sub_pd(_mm_mul_pd( _mm_sub_pd(vvdw12 , _mm_mul_pd(c12_00,_mm_mul_pd(sh_vdw_invrcut6,sh_vdw_invrcut6))),one_twelfth),
	    		       _mm_mul_pd( _mm_sub_pd(vvdw6,_mm_add_pd(_mm_mul_pd(c6_00,sh_vdw_invrcut6),_mm_mul_pd(c6grid_00,sh_lj_ewald))),one_sixth));
            /* fvdw = vvdw12/r - (vvdw6/r + (C6grid * exponent * beta^6)/r) */
            fvdw             = _mm_mul_pd(_mm_sub_pd(vvdw12,_mm_sub_pd(vvdw6,_mm_mul_pd(_mm_mul_pd(c6grid_00,one_sixth),_mm_mul_pd(exponent,ewclj6)))),rinvsq00);

            cutoff_mask      = _mm_cmplt_pd(rsq00,rcutoff2);

            /* Update potential sum for this i atom from the interaction with this j atom. */
            velec            = _mm_and_pd(velec,cutoff_mask);
            velecsum         = _mm_add_pd(velecsum,velec);
            vvdw             = _mm_and_pd(vvdw,cutoff_mask);
            vvdwsum          = _mm_add_pd(vvdwsum,vvdw);

            fscal            = _mm_add_pd(felec,fvdw);

            fscal            = _mm_and_pd(fscal,cutoff_mask);

            /* Calculate temporary vectorial force */
            tx               = _mm_mul_pd(fscal,dx00);
            ty               = _mm_mul_pd(fscal,dy00);
            tz               = _mm_mul_pd(fscal,dz00);

            /* Update vectorial force */
            fix0             = _mm_add_pd(fix0,tx);
            fiy0             = _mm_add_pd(fiy0,ty);
            fiz0             = _mm_add_pd(fiz0,tz);

            gmx_mm_decrement_1rvec_2ptr_swizzle_pd(f+j_coord_offsetA,f+j_coord_offsetB,tx,ty,tz);

            }

            /* Inner loop uses 82 flops */
        }

        if(jidx<j_index_end)
        {

            jnrA             = jjnr[jidx];
            j_coord_offsetA  = DIM*jnrA;

            /* load j atom coordinates */
            gmx_mm_load_1rvec_1ptr_swizzle_pd(x+j_coord_offsetA,
                                              &jx0,&jy0,&jz0);

            /* Calculate displacement vector */
            dx00             = _mm_sub_pd(ix0,jx0);
            dy00             = _mm_sub_pd(iy0,jy0);
            dz00             = _mm_sub_pd(iz0,jz0);

            /* Calculate squared distance and things based on it */
            rsq00            = gmx_mm_calc_rsq_pd(dx00,dy00,dz00);

            rinv00           = sse2_invsqrt_d(rsq00);

            rinvsq00         = _mm_mul_pd(rinv00,rinv00);

            /* Load parameters for j particles */
            jq0              = _mm_load_sd(charge+jnrA+0);
            vdwjidx0A        = 2*vdwtype[jnrA+0];

            /**************************
             * CALCULATE INTERACTIONS *
             **************************/

            if (gmx_mm_any_lt(rsq00,rcutoff2))
            {

            r00              = _mm_mul_pd(rsq00,rinv00);

            /* Compute parameters for interactions between i and j atoms */
            qq00             = _mm_mul_pd(iq0,jq0);
            gmx_mm_load_1pair_swizzle_pd(vdwparam+vdwioffset0+vdwjidx0A,&c6_00,&c12_00);

            c6grid_00       = gmx_mm_load_1real_pd(vdwgridparam+vdwioffset0+vdwjidx0A);

            /* EWALD ELECTROSTATICS */

            /* Calculate Ewald table index by multiplying r with scale and truncate to integer */
            ewrt             = _mm_mul_pd(r00,ewtabscale);
            ewitab           = _mm_cvttpd_epi32(ewrt);
            eweps            = _mm_sub_pd(ewrt,_mm_cvtepi32_pd(ewitab));
            ewitab           = _mm_slli_epi32(ewitab,2);
            ewtabF           = _mm_load_pd( ewtab + gmx_mm_extract_epi32(ewitab,0) );
            ewtabD           = _mm_setzero_pd();
            GMX_MM_TRANSPOSE2_PD(ewtabF,ewtabD);
            ewtabV           = _mm_load_sd( ewtab + gmx_mm_extract_epi32(ewitab,0) +2);
            ewtabFn          = _mm_setzero_pd();
            GMX_MM_TRANSPOSE2_PD(ewtabV,ewtabFn);
            felec            = _mm_add_pd(ewtabF,_mm_mul_pd(eweps,ewtabD));
            velec            = _mm_sub_pd(ewtabV,_mm_mul_pd(_mm_mul_pd(ewtabhalfspace,eweps),_mm_add_pd(ewtabF,felec)));
            velec            = _mm_mul_pd(qq00,_mm_sub_pd(_mm_sub_pd(rinv00,sh_ewald),velec));
            felec            = _mm_mul_pd(_mm_mul_pd(qq00,rinv00),_mm_sub_pd(rinvsq00,felec));

            /* Analytical LJ-PME */
            rinvsix          = _mm_mul_pd(_mm_mul_pd(rinvsq00,rinvsq00),rinvsq00);
            ewcljrsq         = _mm_mul_pd(ewclj2,rsq00);
            ewclj6           = _mm_mul_pd(ewclj2,_mm_mul_pd(ewclj2,ewclj2));
            exponent         = sse2_exp_d(ewcljrsq);
            /* poly = exp(-(beta*r)^2) * (1 + (beta*r)^2 + (beta*r)^4 /2) */
            poly             = _mm_mul_pd(exponent,_mm_add_pd(_mm_sub_pd(one,ewcljrsq),_mm_mul_pd(_mm_mul_pd(ewcljrsq,ewcljrsq),one_half)));
            /* vvdw6 = [C6 - C6grid * (1-poly)]/r6 */
            vvdw6            = _mm_mul_pd(_mm_sub_pd(c6_00,_mm_mul_pd(c6grid_00,_mm_sub_pd(one,poly))),rinvsix);
            vvdw12           = _mm_mul_pd(c12_00,_mm_mul_pd(rinvsix,rinvsix));
            vvdw             = _mm_sub_pd(_mm_mul_pd( _mm_sub_pd(vvdw12 , _mm_mul_pd(c12_00,_mm_mul_pd(sh_vdw_invrcut6,sh_vdw_invrcut6))),one_twelfth),
	    		       _mm_mul_pd( _mm_sub_pd(vvdw6,_mm_add_pd(_mm_mul_pd(c6_00,sh_vdw_invrcut6),_mm_mul_pd(c6grid_00,sh_lj_ewald))),one_sixth));
            /* fvdw = vvdw12/r - (vvdw6/r + (C6grid * exponent * beta^6)/r) */
            fvdw             = _mm_mul_pd(_mm_sub_pd(vvdw12,_mm_sub_pd(vvdw6,_mm_mul_pd(_mm_mul_pd(c6grid_00,one_sixth),_mm_mul_pd(exponent,ewclj6)))),rinvsq00);

            cutoff_mask      = _mm_cmplt_pd(rsq00,rcutoff2);

            /* Update potential sum for this i atom from the interaction with this j atom. */
            velec            = _mm_and_pd(velec,cutoff_mask);
            velec            = _mm_unpacklo_pd(velec,_mm_setzero_pd());
            velecsum         = _mm_add_pd(velecsum,velec);
            vvdw             = _mm_and_pd(vvdw,cutoff_mask);
            vvdw             = _mm_unpacklo_pd(vvdw,_mm_setzero_pd());
            vvdwsum          = _mm_add_pd(vvdwsum,vvdw);

            fscal            = _mm_add_pd(felec,fvdw);

            fscal            = _mm_and_pd(fscal,cutoff_mask);

            fscal            = _mm_unpacklo_pd(fscal,_mm_setzero_pd());

            /* Calculate temporary vectorial force */
            tx               = _mm_mul_pd(fscal,dx00);
            ty               = _mm_mul_pd(fscal,dy00);
            tz               = _mm_mul_pd(fscal,dz00);

            /* Update vectorial force */
            fix0             = _mm_add_pd(fix0,tx);
            fiy0             = _mm_add_pd(fiy0,ty);
            fiz0             = _mm_add_pd(fiz0,tz);

            gmx_mm_decrement_1rvec_1ptr_swizzle_pd(f+j_coord_offsetA,tx,ty,tz);

            }

            /* Inner loop uses 82 flops */
        }

        /* End of innermost loop */

        gmx_mm_update_iforce_1atom_swizzle_pd(fix0,fiy0,fiz0,
                                              f+i_coord_offset,fshift+i_shift_offset);

        ggid                        = gid[iidx];
        /* Update potential energies */
        gmx_mm_update_1pot_pd(velecsum,kernel_data->energygrp_elec+ggid);
        gmx_mm_update_1pot_pd(vvdwsum,kernel_data->energygrp_vdw+ggid);

        /* Increment number of inner iterations */
        inneriter                  += j_index_end - j_index_start;

        /* Outer loop uses 9 flops */
    }

    /* Increment number of outer iterations */
    outeriter        += nri;

    /* Update outer/inner flops */

    inc_nrnb(nrnb,eNR_NBKERNEL_ELEC_VDW_VF,outeriter*9 + inneriter*82);
}
/*
 * Gromacs nonbonded kernel:   nb_kernel_ElecEwSh_VdwLJEwSh_GeomP1P1_F_sse2_double
 * Electrostatics interaction: Ewald
 * VdW interaction:            LJEwald
 * Geometry:                   Particle-Particle
 * Calculate force/pot:        Force
 */
void
nb_kernel_ElecEwSh_VdwLJEwSh_GeomP1P1_F_sse2_double
                    (t_nblist                    * gmx_restrict       nlist,
                     rvec                        * gmx_restrict          xx,
                     rvec                        * gmx_restrict          ff,
                     struct t_forcerec           * gmx_restrict          fr,
                     t_mdatoms                   * gmx_restrict     mdatoms,
                     nb_kernel_data_t gmx_unused * gmx_restrict kernel_data,
                     t_nrnb                      * gmx_restrict        nrnb)
{
    /* Suffixes 0,1,2,3 refer to particle indices for waters in the inner or outer loop, or
     * just 0 for non-waters.
     * Suffixes A,B refer to j loop unrolling done with SSE double precision, e.g. for the two different
     * jnr indices corresponding to data put in the four positions in the SIMD register.
     */
    int              i_shift_offset,i_coord_offset,outeriter,inneriter;
    int              j_index_start,j_index_end,jidx,nri,inr,ggid,iidx;
    int              jnrA,jnrB;
    int              j_coord_offsetA,j_coord_offsetB;
    int              *iinr,*jindex,*jjnr,*shiftidx,*gid;
    real             rcutoff_scalar;
    real             *shiftvec,*fshift,*x,*f;
    __m128d          tx,ty,tz,fscal,rcutoff,rcutoff2,jidxall;
    int              vdwioffset0;
    __m128d          ix0,iy0,iz0,fix0,fiy0,fiz0,iq0,isai0;
    int              vdwjidx0A,vdwjidx0B;
    __m128d          jx0,jy0,jz0,fjx0,fjy0,fjz0,jq0,isaj0;
    __m128d          dx00,dy00,dz00,rsq00,rinv00,rinvsq00,r00,qq00,c6_00,c12_00;
    __m128d          velec,felec,velecsum,facel,crf,krf,krf2;
    real             *charge;
    int              nvdwtype;
    __m128d          rinvsix,rvdw,vvdw,vvdw6,vvdw12,fvdw,fvdw6,fvdw12,vvdwsum,sh_vdw_invrcut6;
    int              *vdwtype;
    real             *vdwparam;
    __m128d          one_sixth   = _mm_set1_pd(1.0/6.0);
    __m128d          one_twelfth = _mm_set1_pd(1.0/12.0);
    __m128d           c6grid_00;
    __m128d           ewclj,ewclj2,ewclj6,ewcljrsq,poly,exponent,f6A,f6B,sh_lj_ewald;
    real             *vdwgridparam;
    __m128d           one_half = _mm_set1_pd(0.5);
    __m128d           minus_one = _mm_set1_pd(-1.0);
    __m128i          ewitab;
    __m128d          ewtabscale,eweps,sh_ewald,ewrt,ewtabhalfspace,ewtabF,ewtabFn,ewtabD,ewtabV;
    real             *ewtab;
    __m128d          dummy_mask,cutoff_mask;
    __m128d          signbit   = gmx_mm_castsi128_pd( _mm_set_epi32(0x80000000,0x00000000,0x80000000,0x00000000) );
    __m128d          one     = _mm_set1_pd(1.0);
    __m128d          two     = _mm_set1_pd(2.0);
    x                = xx[0];
    f                = ff[0];

    nri              = nlist->nri;
    iinr             = nlist->iinr;
    jindex           = nlist->jindex;
    jjnr             = nlist->jjnr;
    shiftidx         = nlist->shift;
    gid              = nlist->gid;
    shiftvec         = fr->shift_vec[0];
    fshift           = fr->fshift[0];
    facel            = _mm_set1_pd(fr->ic->epsfac);
    charge           = mdatoms->chargeA;
    nvdwtype         = fr->ntype;
    vdwparam         = fr->nbfp;
    vdwtype          = mdatoms->typeA;
    vdwgridparam     = fr->ljpme_c6grid;
    sh_lj_ewald      = _mm_set1_pd(fr->ic->sh_lj_ewald);
    ewclj            = _mm_set1_pd(fr->ic->ewaldcoeff_lj);
    ewclj2           = _mm_mul_pd(minus_one,_mm_mul_pd(ewclj,ewclj));

    sh_ewald         = _mm_set1_pd(fr->ic->sh_ewald);
    ewtab            = fr->ic->tabq_coul_F;
    ewtabscale       = _mm_set1_pd(fr->ic->tabq_scale);
    ewtabhalfspace   = _mm_set1_pd(0.5/fr->ic->tabq_scale);

    /* When we use explicit cutoffs the value must be identical for elec and VdW, so use elec as an arbitrary choice */
    rcutoff_scalar   = fr->ic->rcoulomb;
    rcutoff          = _mm_set1_pd(rcutoff_scalar);
    rcutoff2         = _mm_mul_pd(rcutoff,rcutoff);

    sh_vdw_invrcut6  = _mm_set1_pd(fr->ic->sh_invrc6);
    rvdw             = _mm_set1_pd(fr->ic->rvdw);

    /* Avoid stupid compiler warnings */
    jnrA = jnrB = 0;
    j_coord_offsetA = 0;
    j_coord_offsetB = 0;

    outeriter        = 0;
    inneriter        = 0;

    /* Start outer loop over neighborlists */
    for(iidx=0; iidx<nri; iidx++)
    {
        /* Load shift vector for this list */
        i_shift_offset   = DIM*shiftidx[iidx];

        /* Load limits for loop over neighbors */
        j_index_start    = jindex[iidx];
        j_index_end      = jindex[iidx+1];

        /* Get outer coordinate index */
        inr              = iinr[iidx];
        i_coord_offset   = DIM*inr;

        /* Load i particle coords and add shift vector */
        gmx_mm_load_shift_and_1rvec_broadcast_pd(shiftvec+i_shift_offset,x+i_coord_offset,&ix0,&iy0,&iz0);

        fix0             = _mm_setzero_pd();
        fiy0             = _mm_setzero_pd();
        fiz0             = _mm_setzero_pd();

        /* Load parameters for i particles */
        iq0              = _mm_mul_pd(facel,_mm_load1_pd(charge+inr+0));
        vdwioffset0      = 2*nvdwtype*vdwtype[inr+0];

        /* Start inner kernel loop */
        for(jidx=j_index_start; jidx<j_index_end-1; jidx+=2)
        {

            /* Get j neighbor index, and coordinate index */
            jnrA             = jjnr[jidx];
            jnrB             = jjnr[jidx+1];
            j_coord_offsetA  = DIM*jnrA;
            j_coord_offsetB  = DIM*jnrB;

            /* load j atom coordinates */
            gmx_mm_load_1rvec_2ptr_swizzle_pd(x+j_coord_offsetA,x+j_coord_offsetB,
                                              &jx0,&jy0,&jz0);

            /* Calculate displacement vector */
            dx00             = _mm_sub_pd(ix0,jx0);
            dy00             = _mm_sub_pd(iy0,jy0);
            dz00             = _mm_sub_pd(iz0,jz0);

            /* Calculate squared distance and things based on it */
            rsq00            = gmx_mm_calc_rsq_pd(dx00,dy00,dz00);

            rinv00           = sse2_invsqrt_d(rsq00);

            rinvsq00         = _mm_mul_pd(rinv00,rinv00);

            /* Load parameters for j particles */
            jq0              = gmx_mm_load_2real_swizzle_pd(charge+jnrA+0,charge+jnrB+0);
            vdwjidx0A        = 2*vdwtype[jnrA+0];
            vdwjidx0B        = 2*vdwtype[jnrB+0];

            /**************************
             * CALCULATE INTERACTIONS *
             **************************/

            if (gmx_mm_any_lt(rsq00,rcutoff2))
            {

            r00              = _mm_mul_pd(rsq00,rinv00);

            /* Compute parameters for interactions between i and j atoms */
            qq00             = _mm_mul_pd(iq0,jq0);
            gmx_mm_load_2pair_swizzle_pd(vdwparam+vdwioffset0+vdwjidx0A,
                                         vdwparam+vdwioffset0+vdwjidx0B,&c6_00,&c12_00);

            c6grid_00       = gmx_mm_load_2real_swizzle_pd(vdwgridparam+vdwioffset0+vdwjidx0A,
                                                               vdwgridparam+vdwioffset0+vdwjidx0B);

            /* EWALD ELECTROSTATICS */

            /* Calculate Ewald table index by multiplying r with scale and truncate to integer */
            ewrt             = _mm_mul_pd(r00,ewtabscale);
            ewitab           = _mm_cvttpd_epi32(ewrt);
            eweps            = _mm_sub_pd(ewrt,_mm_cvtepi32_pd(ewitab));
            gmx_mm_load_2pair_swizzle_pd(ewtab+gmx_mm_extract_epi32(ewitab,0),ewtab+gmx_mm_extract_epi32(ewitab,1),
                                         &ewtabF,&ewtabFn);
            felec            = _mm_add_pd(_mm_mul_pd( _mm_sub_pd(one,eweps),ewtabF),_mm_mul_pd(eweps,ewtabFn));
            felec            = _mm_mul_pd(_mm_mul_pd(qq00,rinv00),_mm_sub_pd(rinvsq00,felec));

            /* Analytical LJ-PME */
            rinvsix          = _mm_mul_pd(_mm_mul_pd(rinvsq00,rinvsq00),rinvsq00);
            ewcljrsq         = _mm_mul_pd(ewclj2,rsq00);
            ewclj6           = _mm_mul_pd(ewclj2,_mm_mul_pd(ewclj2,ewclj2));
            exponent         = sse2_exp_d(ewcljrsq);
            /* poly = exp(-(beta*r)^2) * (1 + (beta*r)^2 + (beta*r)^4 /2) */
            poly             = _mm_mul_pd(exponent,_mm_add_pd(_mm_sub_pd(one,ewcljrsq),_mm_mul_pd(_mm_mul_pd(ewcljrsq,ewcljrsq),one_half)));
            /* f6A = 6 * C6grid * (1 - poly) */
            f6A              = _mm_mul_pd(c6grid_00,_mm_sub_pd(one,poly));
            /* f6B = C6grid * exponent * beta^6 */
            f6B              = _mm_mul_pd(_mm_mul_pd(c6grid_00,one_sixth),_mm_mul_pd(exponent,ewclj6));
            /* fvdw = 12*C12/r13 - ((6*C6 - f6A)/r6 + f6B)/r */
	    fvdw              = _mm_mul_pd(_mm_add_pd(_mm_mul_pd(_mm_sub_pd(_mm_mul_pd(c12_00,rinvsix),_mm_sub_pd(c6_00,f6A)),rinvsix),f6B),rinvsq00);

            cutoff_mask      = _mm_cmplt_pd(rsq00,rcutoff2);

            fscal            = _mm_add_pd(felec,fvdw);

            fscal            = _mm_and_pd(fscal,cutoff_mask);

            /* Calculate temporary vectorial force */
            tx               = _mm_mul_pd(fscal,dx00);
            ty               = _mm_mul_pd(fscal,dy00);
            tz               = _mm_mul_pd(fscal,dz00);

            /* Update vectorial force */
            fix0             = _mm_add_pd(fix0,tx);
            fiy0             = _mm_add_pd(fiy0,ty);
            fiz0             = _mm_add_pd(fiz0,tz);

            gmx_mm_decrement_1rvec_2ptr_swizzle_pd(f+j_coord_offsetA,f+j_coord_offsetB,tx,ty,tz);

            }

            /* Inner loop uses 62 flops */
        }

        if(jidx<j_index_end)
        {

            jnrA             = jjnr[jidx];
            j_coord_offsetA  = DIM*jnrA;

            /* load j atom coordinates */
            gmx_mm_load_1rvec_1ptr_swizzle_pd(x+j_coord_offsetA,
                                              &jx0,&jy0,&jz0);

            /* Calculate displacement vector */
            dx00             = _mm_sub_pd(ix0,jx0);
            dy00             = _mm_sub_pd(iy0,jy0);
            dz00             = _mm_sub_pd(iz0,jz0);

            /* Calculate squared distance and things based on it */
            rsq00            = gmx_mm_calc_rsq_pd(dx00,dy00,dz00);

            rinv00           = sse2_invsqrt_d(rsq00);

            rinvsq00         = _mm_mul_pd(rinv00,rinv00);

            /* Load parameters for j particles */
            jq0              = _mm_load_sd(charge+jnrA+0);
            vdwjidx0A        = 2*vdwtype[jnrA+0];

            /**************************
             * CALCULATE INTERACTIONS *
             **************************/

            if (gmx_mm_any_lt(rsq00,rcutoff2))
            {

            r00              = _mm_mul_pd(rsq00,rinv00);

            /* Compute parameters for interactions between i and j atoms */
            qq00             = _mm_mul_pd(iq0,jq0);
            gmx_mm_load_1pair_swizzle_pd(vdwparam+vdwioffset0+vdwjidx0A,&c6_00,&c12_00);

            c6grid_00       = gmx_mm_load_1real_pd(vdwgridparam+vdwioffset0+vdwjidx0A);

            /* EWALD ELECTROSTATICS */

            /* Calculate Ewald table index by multiplying r with scale and truncate to integer */
            ewrt             = _mm_mul_pd(r00,ewtabscale);
            ewitab           = _mm_cvttpd_epi32(ewrt);
            eweps            = _mm_sub_pd(ewrt,_mm_cvtepi32_pd(ewitab));
            gmx_mm_load_1pair_swizzle_pd(ewtab+gmx_mm_extract_epi32(ewitab,0),&ewtabF,&ewtabFn);
            felec            = _mm_add_pd(_mm_mul_pd( _mm_sub_pd(one,eweps),ewtabF),_mm_mul_pd(eweps,ewtabFn));
            felec            = _mm_mul_pd(_mm_mul_pd(qq00,rinv00),_mm_sub_pd(rinvsq00,felec));

            /* Analytical LJ-PME */
            rinvsix          = _mm_mul_pd(_mm_mul_pd(rinvsq00,rinvsq00),rinvsq00);
            ewcljrsq         = _mm_mul_pd(ewclj2,rsq00);
            ewclj6           = _mm_mul_pd(ewclj2,_mm_mul_pd(ewclj2,ewclj2));
            exponent         = sse2_exp_d(ewcljrsq);
            /* poly = exp(-(beta*r)^2) * (1 + (beta*r)^2 + (beta*r)^4 /2) */
            poly             = _mm_mul_pd(exponent,_mm_add_pd(_mm_sub_pd(one,ewcljrsq),_mm_mul_pd(_mm_mul_pd(ewcljrsq,ewcljrsq),one_half)));
            /* f6A = 6 * C6grid * (1 - poly) */
            f6A              = _mm_mul_pd(c6grid_00,_mm_sub_pd(one,poly));
            /* f6B = C6grid * exponent * beta^6 */
            f6B              = _mm_mul_pd(_mm_mul_pd(c6grid_00,one_sixth),_mm_mul_pd(exponent,ewclj6));
            /* fvdw = 12*C12/r13 - ((6*C6 - f6A)/r6 + f6B)/r */
	    fvdw              = _mm_mul_pd(_mm_add_pd(_mm_mul_pd(_mm_sub_pd(_mm_mul_pd(c12_00,rinvsix),_mm_sub_pd(c6_00,f6A)),rinvsix),f6B),rinvsq00);

            cutoff_mask      = _mm_cmplt_pd(rsq00,rcutoff2);

            fscal            = _mm_add_pd(felec,fvdw);

            fscal            = _mm_and_pd(fscal,cutoff_mask);

            fscal            = _mm_unpacklo_pd(fscal,_mm_setzero_pd());

            /* Calculate temporary vectorial force */
            tx               = _mm_mul_pd(fscal,dx00);
            ty               = _mm_mul_pd(fscal,dy00);
            tz               = _mm_mul_pd(fscal,dz00);

            /* Update vectorial force */
            fix0             = _mm_add_pd(fix0,tx);
            fiy0             = _mm_add_pd(fiy0,ty);
            fiz0             = _mm_add_pd(fiz0,tz);

            gmx_mm_decrement_1rvec_1ptr_swizzle_pd(f+j_coord_offsetA,tx,ty,tz);

            }

            /* Inner loop uses 62 flops */
        }

        /* End of innermost loop */

        gmx_mm_update_iforce_1atom_swizzle_pd(fix0,fiy0,fiz0,
                                              f+i_coord_offset,fshift+i_shift_offset);

        /* Increment number of inner iterations */
        inneriter                  += j_index_end - j_index_start;

        /* Outer loop uses 7 flops */
    }

    /* Increment number of outer iterations */
    outeriter        += nri;

    /* Update outer/inner flops */

    inc_nrnb(nrnb,eNR_NBKERNEL_ELEC_VDW_F,outeriter*7 + inneriter*62);
}
