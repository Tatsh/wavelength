#pragma once

#include "game/triggeraction.h"
#include "script/dataarray.h"

/**
 * An action that runs its node as a script command.
 *
 * The RTTI records the class as deriving from TriggerAction. The object is 0x10 bytes.
 * TriggerAction::New() builds one for every node whose name is not a kind of action it knows.
 */
class DataFuncAction : public TriggerAction {
public:
    /**
     * Keep a reference on the node.
     *
     * @param pAction The node, the command.
     * @ghidraAddress NTSC-U/C: 0x00200700
     * @ghidraAddress PAL: 0x002094a0
     */
    explicit DataFuncAction(DataArray *pAction);

    ~DataFuncAction() override {
        mCommand->Release();
    }

    /**
     * Run the command through ScriptFunction::Dispatch().
     *
     * @ghidraAddress NTSC-U/C: 0x00202498
     * @ghidraAddress PAL: 0x0020b238
     */
    void Exec() override;

private:
    DataArray *mCommand;
};
