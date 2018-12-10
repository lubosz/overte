"use strict";

//  nearGrabEntity.js
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html


/* global Script, Entities, MyAvatar, Controller, RIGHT_HAND, LEFT_HAND, getControllerJointIndex, enableDispatcherModule,
   disableDispatcherModule, Messages, HAPTIC_PULSE_STRENGTH, HAPTIC_PULSE_DURATION, TRIGGER_OFF_VALUE, TRIGGER_ON_VALUE,
   makeDispatcherModuleParameters, entityIsGrabbable, makeRunningValues, NEAR_GRAB_RADIUS, findGroupParent, Vec3,
   cloneEntity, entityIsCloneable, HAPTIC_PULSE_STRENGTH, HAPTIC_PULSE_DURATION, BUMPER_ON_VALUE, TEAR_AWAY_DISTANCE,
   distanceBetweenPointAndEntityBoundingBox, highlightTargetEntity, unhighlightTargetEntity, getGrabbableData,
   DISPATCHER_PROPERTIES, HMD
*/

Script.include("/~/system/libraries/controllerDispatcherUtils.js");
Script.include("/~/system/libraries/cloneEntityUtils.js");
Script.include("/~/system/libraries/controllers.js");

(function() {

    function NearGrabEntity(hand) {
        this.hand = hand;
        this.targetEntityID = null;
        this.grabbing = false;
        this.hapticTargetID = null;
        this.highlightedEntity = null;
        this.cloneAllowed = true;
        this.grabID = null;

        this.parameters = makeDispatcherModuleParameters(
            500,
            this.hand === RIGHT_HAND ? ["rightHand"] : ["leftHand"],
            [],
            100);

        this.startNearGrabEntity = function (controllerData, targetProps) {
            var grabData = getGrabbableData(targetProps);
            Controller.triggerHapticPulse(HAPTIC_PULSE_STRENGTH, HAPTIC_PULSE_DURATION, this.hand);
            unhighlightTargetEntity(this.targetEntityID);
            this.highlightedEntity = null;
            var message = {
                hand: this.hand,
                entityID: this.targetEntityID
            };

            Messages.sendLocalMessage('Hifi-unhighlight-entity', JSON.stringify(message));
            var handJointIndex;
            if (HMD.mounted && HMD.isHandControllerAvailable() && grabData.grabFollowsController) {
                handJointIndex = getControllerJointIndex(this.hand);
            } else {
                handJointIndex = MyAvatar.getJointIndex(this.hand === RIGHT_HAND ? "RightHand" : "LeftHand");
            }

            var args = [this.hand === RIGHT_HAND ? "right" : "left", MyAvatar.sessionUUID];
            Entities.callEntityMethod(targetProps.id, "startNearGrab", args);

            this.targetEntityID = targetProps.id;

            if (this.grabID) {
                MyAvatar.releaseGrab(this.grabID);
            }

            var relativePosition = Entities.worldToLocalPosition(targetProps.position, MyAvatar.SELF_ID, handJointIndex);
            var relativeRotation = Entities.worldToLocalRotation(targetProps.rotation, MyAvatar.SELF_ID, handJointIndex);

            this.grabID = MyAvatar.grab(targetProps.id, handJointIndex, relativePosition, relativeRotation);

            Messages.sendMessage('Hifi-Object-Manipulation', JSON.stringify({
                action: 'grab',
                grabbedEntity: targetProps.id,
                joint: this.hand === RIGHT_HAND ? "RightHand" : "LeftHand"
            }));
            this.grabbing = true;
        };

        this.endNearGrabEntity = function (controllerData) {
            if (this.grabID) {
                MyAvatar.releaseGrab(this.grabID);
                this.grabID = null;
            }

            this.hapticTargetID = null;

            var args = [this.hand === RIGHT_HAND ? "right" : "left", MyAvatar.sessionUUID];
            Entities.callEntityMethod(this.targetEntityID, "releaseGrab", args);
            Messages.sendMessage('Hifi-Object-Manipulation', JSON.stringify({
                action: 'release',
                grabbedEntity: this.targetEntityID,
                joint: this.hand === RIGHT_HAND ? "RightHand" : "LeftHand"
            }));
            unhighlightTargetEntity(this.targetEntityID);
            this.highlightedEntity = null;
            this.grabbing = false;
            this.targetEntityID = null;
        };

        this.getTargetProps = function (controllerData) {
            // nearbyEntityProperties is already sorted by length from controller
            var nearbyEntityProperties = controllerData.nearbyEntityProperties[this.hand];
            var sensorScaleFactor = MyAvatar.sensorToWorldScale;
            for (var i = 0; i < nearbyEntityProperties.length; i++) {
                var props = nearbyEntityProperties[i];
                var handPosition = controllerData.controllerLocations[this.hand].position;
                var dist = distanceBetweenPointAndEntityBoundingBox(handPosition, props);
                var distance = Vec3.distance(handPosition, props.position);
                if ((dist > TEAR_AWAY_DISTANCE) ||
                    (distance > NEAR_GRAB_RADIUS * sensorScaleFactor)) {
                    continue;
                }
                if (entityIsGrabbable(props) || entityIsCloneable(props)) {
                    // give haptic feedback
                    if (props.id !== this.hapticTargetID) {
                        Controller.triggerHapticPulse(HAPTIC_PULSE_STRENGTH, HAPTIC_PULSE_DURATION, this.hand);
                        this.hapticTargetID = props.id;
                    }
                    if (!entityIsCloneable(props)) {
                        // if we've attempted to grab a non-cloneable child, roll up to the root of the tree
                        var groupRootProps = findGroupParent(controllerData, props);
                        if (entityIsGrabbable(groupRootProps)) {
                            return groupRootProps;
                        }
                    }
                    return props;
                }
            }
            return null;
        };

        this.isReady = function (controllerData, deltaTime) {
            this.targetEntityID = null;
            this.grabbing = false;

            if (controllerData.triggerValues[this.hand] < TRIGGER_OFF_VALUE &&
                controllerData.secondaryValues[this.hand] < TRIGGER_OFF_VALUE) {
                this.cloneAllowed = true;
                return makeRunningValues(false, [], []);
            }

            var targetProps = this.getTargetProps(controllerData);
            if (targetProps) {
                this.targetEntityID = targetProps.id;
                this.highlightedEntity = this.targetEntityID;
                highlightTargetEntity(this.targetEntityID);
                return makeRunningValues(true, [this.targetEntityID], []);
            } else {
                if (this.highlightedEntity) {
                    unhighlightTargetEntity(this.highlightedEntity);
                    this.highlightedEntity = null;
                }
                this.hapticTargetID = null;
                return makeRunningValues(false, [], []);
            }
        };

        this.run = function (controllerData, deltaTime) {
            if (this.grabbing) {
                if (controllerData.triggerClicks[this.hand] < TRIGGER_OFF_VALUE &&
                    controllerData.secondaryValues[this.hand] < TRIGGER_OFF_VALUE) {
                    this.endNearGrabEntity(controllerData);
                    return makeRunningValues(false, [], []);
                }

                if (controllerData.secondaryValues[LEFT_HAND] >= TRIGGER_ON_VALUE &&
                    controllerData.secondaryValues[RIGHT_HAND] >= TRIGGER_ON_VALUE) {
                    // let scaleEntity module take over
                    this.endNearGrabEntity(controllerData);
                    return makeRunningValues(false, [], []);
                }

                var props = controllerData.nearbyEntityPropertiesByID[this.targetEntityID];
                if (!props) {
                    props = Entities.getEntityProperties(this.targetEntityID, DISPATCHER_PROPERTIES);
                    if (!props) {
                        // entity was deleted
                        unhighlightTargetEntity(this.targetEntityID);
                        this.highlightedEntity = null;
                        this.grabbing = false;
                        this.targetEntityID = null;
                        this.hapticTargetID = null;
                        return makeRunningValues(false, [], []);
                    }
                }

                var args = [this.hand === RIGHT_HAND ? "right" : "left", MyAvatar.sessionUUID];
                Entities.callEntityMethod(this.targetEntityID, "continueNearGrab", args);
            } else {
                // still searching / highlighting
                var readiness = this.isReady(controllerData);
                if (!readiness.active) {
                    unhighlightTargetEntity(this.highlightedEntity);
                    this.highlightedEntity = null;
                    return readiness;
                }
                if (controllerData.triggerClicks[this.hand] || controllerData.secondaryValues[this.hand] > BUMPER_ON_VALUE) {
                    // switch to grab
                    var targetProps = this.getTargetProps(controllerData);
                    var targetCloneable = entityIsCloneable(targetProps);

                    if (targetCloneable) {
                        if (this.cloneAllowed) {
                            var cloneID = cloneEntity(targetProps);
                            if (cloneID !== null) {
                                var cloneProps = Entities.getEntityProperties(cloneID, DISPATCHER_PROPERTIES);
                                this.grabbing = true;
                                this.targetEntityID = cloneID;
                                this.startNearGrabEntity(controllerData, cloneProps);
                                this.cloneAllowed = false; // prevent another clone call until inputs released
                            }
                        }
                    } else if (targetProps) {
                        this.grabbing = true;
                        this.startNearGrabEntity(controllerData, targetProps);
                    }
                }
            }

            return makeRunningValues(true, [this.targetEntityID], []);
        };

        this.cleanup = function () {
            if (this.targetEntityID) {
                this.endNearGrabEntity();
            }
        };
    }

    var leftNearGrabEntity = new NearGrabEntity(LEFT_HAND);
    var rightNearGrabEntity = new NearGrabEntity(RIGHT_HAND);

    enableDispatcherModule("LeftNearGrabEntity", leftNearGrabEntity);
    enableDispatcherModule("RightNearGrabEntity", rightNearGrabEntity);

    function cleanup() {
        leftNearGrabEntity.cleanup();
        rightNearGrabEntity.cleanup();
        disableDispatcherModule("LeftNearGrabEntity");
        disableDispatcherModule("RightNearGrabEntity");
    }
    Script.scriptEnding.connect(cleanup);
}());
