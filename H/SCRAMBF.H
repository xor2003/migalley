//------------------------------------------------------------------------------
//Filename       scrambf.h
//System         
//Author         R. Hyde 
//Date           Sun 25 Aug 1996
//Description    
//------------------------------------------------------------------------------
#ifndef	SCRAMBF_Included
#define	SCRAMBF_Included

// RERUN: self-assert packing - DOSDEFS-era headers assumed ambient pack(1);
// guard so include order cannot change this layout across TUs (ODR).
#pragma pack(push,1)

#define	DEFAULT_SCRAMBF 0



	enum	{BF_FirstFlightBrit = 0,
				BF_FollowLdrBrit,
				BF_TurkeyShootBrit,
				BF_GroundAttackBrit,
				BF_OneOnOneIIBrit,
				BF_SquadEncounterIIBrit,
				BF_StalkingPreyBrit,
				BF_FirstPatrolBrit,
				BF_NumMissions = BF_FirstPatrolBrit,
				BF_FirstFlightGerm,
				BF_FollowLdrGerm,
				BF_TurkeyShootGerm,
				BF_GroundAttackGerm,
				BF_OneOnOneIIGerm,
				BF_SquadEncounterIIGerm,
				BF_StalkingPreyGerm,
				BF_FirstPatrolGerm,


				BF_OneOnOne,
				BF_OverRoucourt,
				BF_surprise,
				BF_OverHavrincourt,
				BF_SquadEncounter};

#pragma pack(pop)
#endif
