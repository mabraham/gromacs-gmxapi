//
// Created by Eric Irrgang on 9/23/17.
//

#ifndef GROMACS_RESTRAINT_RESTRAINTPOTENTIAL_H
#define GROMACS_RESTRAINT_RESTRAINTPOTENTIAL_H

/*!
 * \defgroup module_restraint MD restraints
 * \ingroup publicapi
 * \brief Apply restraints during MD integration.
 *
 * More conceptual overview is in the \ref page_pullcodeextension "full documentation".
 *
 * The classes here are available through the public API, but only gmx::RestraintPotential
 * is necessary to implement a restraint plugin.
 */
/*! \file
 * \brief Declare generic interface for restraint implementations.
 *
 * \ingroup module_restraint
 */

#include "vectortype.h"

#include <memory>
#include <vector>
#include <functional>
#include <ostream>

#include "gromacs/math/vectypes.h"
#include "gromacs/utility/basedefinitions.h"
#include "gromacs/utility/real.h"

struct gmx_mtop_t;
struct gmx_output_env_t;
struct pull_params_t;
struct pull_t;
struct t_commrec;
struct t_filenm;
struct t_inputrec;
struct t_mdatoms;
struct t_pbc;

namespace gmxapi
{
class Session;
class SessionResources;
}

namespace gmx
{

/*!
 * \brief Provide a vector type name with a more stable interface than RVec and a more stable
 * implementation than vec3<>.
 *
 * \ingroup module_restraint
 *
 * \internal
 * Type alias is used at namespace level
 */
using Vector = ::gmx::detail::vec3<real>;

/*!
 * \brief Structure to hold the results of IRestraintPotential::evaluate().
 *
 * \ingroup module_restraint
 */
class PotentialPointData
{
    public:
        /*!
         * \brief Force vector calculated for first position.
         */
        Vector force;
        /*!
         * \brief Potential energy calculated for this interaction.
         */
        real energy;


        /*!
         * \brief Initialize a new data structure.
         */
        PotentialPointData() : PotentialPointData{Vector(), real(0.0)}
        {};

        /*!
         * \brief Initialize from an argument list
         *
         * \param f Force vector.
         * \param e Energy value.
         *
         * Note that if force was calculated as a scalar, it needs to be multiplied by a unit
         * vector in the direction to which it should be applied.
         * If this calculation is in a subclass of gmx::RestraintPotential,
         * you should be able to use the make_force_vec() helper function (not yet implemented).
         */
        PotentialPointData(const Vector &f, const real e) :
            force {f},
        energy {e}
        {};
};

/*!
 * \brief Interface for Restraint potentials.
 *
 * Derive from this interface class to implement a restraint potential. The derived class
 * must implement the evaluate() member function that produces a PotentialPointData instance.
 * For various convenience functions and to decouple the internal library
 * interface from implementations of potentials, it is expected that
 * restraints will be programmed by subclassing gmx::RestraintPotential<>
 * rather than gmx::IRestraintPotential.
 *
 * For a set of \f$n\f$ coordinates, generate a force field according to a
 * scalar potential that is a fun. \f$F_i = - \nabla_{q_i} \Phi (q_0, q_1, ... q_n; t)\f$
 *
 * Potentials implemented with these classes may be long ranged and are appropriate
 * for only a small number of particles to avoid substantial performance impact.
 *
 * The indices in the above equation refer to the input and output sites specified before the simulation is started.
 * In the current interface, the evaluate() virtual function allows an
 * implementer to calculate the energy and force acting at the first of a pair of sites. The equal and opposite force is applied at the second site.
 *
 * The potential function is evaluated with a time argument
 * and can be updated during the simulation.
 * For non-time-varying potentials, the time argument may still be useful for
 * internal optimizations, such as managing cached values.
 *
 * In the simplest and most common case, pairs of sites (atoms or groups)
 * are specified by the user and then, during integration, GROMACS provides
 * the current positions of each pair for the restraint potential to be evaluated.
 * In such a case, the potential can be implemented by overriding...
 *
 * \ingroup module_restraint
 */
class IRestraintPotential
{
    public:
        virtual ~IRestraintPotential() = default;

        /*!
         * \brief Calculate a force vector according to two input positions at a given time.
         *
         * If not overridden by derived class, returns a zero vector.
         * \param r1 position of first site
         * \param r2 position of second site
         * \param t simulation time in picoseconds
         * \return force vector and potential energy to be applied by calling code.
         *
         * \todo The virtual function call should be replaced by a (table of) function objects retrieved before the run.
         */
        virtual PotentialPointData evaluate(Vector r1,
                                            Vector r2,
                                            double t) = 0;


        /*!
         * \brief Call-back hook for restraint implementations.
         *
         * An update function to be called on the simulation master rank/thread periodically
         * by the Restraint framework.
         * Receives the same input as the evaluate() method, but is only called on the master
         * rank of a simulation to allow implementation code to be thread-safe without knowing
         * anything about the domain decomposition.
         *
         * \param v position of the first site
         * \param v0 position of the second site
         * \param t simulation time
         *
         * \internal
         * We give the definition here because we don't want plugins to have to link against
         * libgromacs right now (complicated header maintenance and no API stability guarantees).
         * But once we've had plugin restraints wrap themselves in a Restraint template, we can set update = 0
         * \todo: Provide gmxapi facility for plugin restraints to wrap themselves with a default implementation to let this class be pure virtual.
         */
        virtual void update(gmx::Vector v,
                            gmx::Vector v0,
                            double      t) { (void)v; (void)v0; (void)t; };


        /*!
         * \brief Find out what sites this restraint is configured to act on.
         * \return
         */
        virtual
        std::vector<unsigned long int> sites() const = 0;

        /*!
         * \brief Allow Session-mediated interaction with other resources or workflow elements.
         *
         * \param resources temporary access to the resources provided by the session for additional configuration.
         *
         * A module implements this method to receive a handle to resources configured for this particular workflow
         * element.
         *
         * \internal
         * \todo This should be more general than the RestraintPotential interface.
         */
        virtual void bindSession(gmxapi::SessionResources* resources)
        {
            (void) resources;
        }
};

} // end namespace gmx

#endif //GMX_PULLING_PULLPOTENTIAL_H