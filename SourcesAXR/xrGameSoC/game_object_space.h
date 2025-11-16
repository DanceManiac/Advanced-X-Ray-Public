#pragma once

namespace GameObject {
	enum ECallbackType {
		eTradeStart = u32(0),
		eTradeStop,
		eTradeSellBuyItem,
		eTradePerformTradeOperation,

		eZoneEnter,
		eZoneExit,
		eExitLevelBorder,
		eEnterLevelBorder,
		eDeath,

		ePatrolPathInPoint,

		eInventoryPda,
		eInventoryInfo,
		eArticleInfo,
		eTaskStateChange,
		eMapLocationAdded,

		eUseObject,

		eHit,

		eSound,

		eActionTypeMovement,
		eActionTypeWatch,
		eActionTypeAnimation,
		eActionTypeSound,
		eActionTypeParticle,
		eActionTypeObject,
		eActionTypeWeaponFire,

		eActorSleep,

		eHelicopterOnPoint,
		eHelicopterOnHit,

		eOnItemTake,
		eOnItemDrop,

		eScriptAnimation,
		
		eTraderGlobalAnimationRequest,
		eTraderHeadAnimationRequest,
		eTraderSoundEnd,

		eInvBoxItemTake,

		//Alundaio: added defines
		eActorHudAnimationEnd,
		//AVO: custom callbacks

		// input
		eKeyPress,
		eKeyRelease,
		eKeyHold,

		eMouseMove,
		eMouseWheel,

		// inventory
		eItemToBelt,
		eItemToSlot,
		eItemToRuck,

		// weapon
		eOnWeaponZoomIn,
		eOnWeaponZoomOut,
		eOnWeaponJammed,
		eOnWeaponFired,
		eOnWeaponMagazineEmpty,

		// vehicle
		eAttachVehicle,
		eDetachVehicle,
		eUseVehicle,

		eOnFootStep,
		//-AVO

		//Dance Maniac
		eOnActorJump,
		//-Dance Maniac

		eOnActorLand,

		//eOnWeaponLowered,	 //For safemode
		//eOnWeaponRaised,	 //For safemode

		eDummy = u32(-1),
	};
};