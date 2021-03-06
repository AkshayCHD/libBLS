/*
  Copyright (C) 2018-2019 SKALE Labs

  This file is part of libBLS.

  libBLS is free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as published
  by the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  libBLS is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Affero General Public License for more details.

  You should have received a copy of the GNU Affero General Public License
  along with libBLS.  If not, see <https://www.gnu.org/licenses/>.

  @file BLSSigShare.cpp
  @author Stan Kladko, Sveta Rogova
  @date 2019
*/

#include <bls/BLSutils.h>
#include <bls/BLSSigShare.h>
#include <bls/BLSSignature.h>

#include <stdlib.h>
#include <string>

std::shared_ptr<libff::alt_bn128_G1> BLSSigShare::getSigShare() const {
  return sigShare;
}
size_t BLSSigShare::getSignerIndex() const {
  return signerIndex;
}

std::shared_ptr<std::string> BLSSigShare::toString() {
  char str[512];

  sigShare->to_affine_coordinates();

  gmp_sprintf(str, "%Nd:%Nd:%s", sigShare->X.as_bigint().data, libff::alt_bn128_Fq::num_limbs,
      sigShare->Y.as_bigint().data, libff::alt_bn128_Fq::num_limbs, hint.c_str());

  return std::make_shared<std::string>(str);
}

BLSSigShare::BLSSigShare(std::shared_ptr<std::string> _sigShare, size_t _signerIndex, size_t _requiredSigners,
        size_t _totalSigners)
    : signerIndex(_signerIndex),
      totalSigners(_totalSigners),
      requiredSigners(_requiredSigners) {
  BLSSignature::checkSigners(requiredSigners, totalSigners);
  BLSutils::initBLS();
  if (signerIndex == 0) {
    BOOST_THROW_EXCEPTION(std::runtime_error("Zero signer index"));
  }

  if (!_sigShare) {
    BOOST_THROW_EXCEPTION(std::runtime_error("Null _sigShare"));
  }


  if (_sigShare->size() < 10) {
    BOOST_THROW_EXCEPTION(
          std::runtime_error("Signature too short:" + std::to_string(_sigShare->size())));
  }

  if ( _sigShare->size() > BLS_MAX_SIG_LEN ) {
    BOOST_THROW_EXCEPTION(std::runtime_error( "Signature too long:" + std::to_string( _sigShare->size() ) ) );
  }


  std::shared_ptr<std::vector<std::string>> result = BLSutils::SplitString( _sigShare, ":");
  if ( result->size() != 4 )
      BOOST_THROW_EXCEPTION(std::runtime_error("Misformatted signature"));
  for ( auto && str : *result){
      for ( char& c : str ) {
          if ( !( c >= '0' && c <= '9' ) ) {
              BOOST_THROW_EXCEPTION(std::runtime_error(
                                             "Misformatted char:" + std::to_string( ( int ) c ) + " in component " +  str ) );
          }
      }
  }

    libff::alt_bn128_Fq X(result->at(0).c_str());
    libff::alt_bn128_Fq Y(result->at(1).c_str());

    sigShare = std::make_shared< libff::alt_bn128_G1 >( X, Y,libff::alt_bn128_Fq::one());
    hint = result->at(2) + ":" + result->at(3);

    if ( !(*sigShare).is_well_formed() )
      BOOST_THROW_EXCEPTION(std::runtime_error("signature is not from G1"));
}

BLSSigShare::BLSSigShare( const std::shared_ptr< libff::alt_bn128_G1 >& _sigShare, std::string & _hint,  size_t _signerIndex,
                          size_t _requiredSigners, size_t _totalSigners )
    : sigShare( _sigShare ),
      hint (_hint),
      signerIndex( _signerIndex ),
      requiredSigners( _requiredSigners ),
      totalSigners( _totalSigners ) {

    BLSSignature::checkSigners( requiredSigners, totalSigners );
    if (  _sigShare->is_zero() ) {
        BOOST_THROW_EXCEPTION(std::runtime_error( "Zero signature" ) );
    }
    if ( _signerIndex == 0 ) {
      BOOST_THROW_EXCEPTION(std::runtime_error( "Zero signer index" ) );
    }

    if ( !_sigShare ) {
        BOOST_THROW_EXCEPTION(std::runtime_error( "Null _s" ) );
    }
    if ( hint.length() == 0 ) {
      BOOST_THROW_EXCEPTION(std::runtime_error( "Empty or misformatted hint" ) );
    }
}

size_t BLSSigShare::getTotalSigners() const {
  return totalSigners;
}
size_t BLSSigShare::getRequiredSigners() const {
  return requiredSigners;
}

std::string BLSSigShare::getHint() const {
  return hint;
}
