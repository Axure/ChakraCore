
//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
#pragma once

#if ENABLE_TTD

namespace TTD
{
    namespace JsSupport
    {
        //return true if the Var is a tagged number (inline)
        bool IsVarTaggedInline(Js::Var v);

        //return true if the Var is a ptr var value 
        bool IsVarPtrValued(Js::Var v);

        //return true if the Var is a primitive value (string, number, symbol, etc.)
        bool IsVarPrimitiveKind(Js::Var v);

        //return true if the Var is a richer value (enumerator, dynamicObject, array, etc.)
        bool IsVarComplexKind(Js::Var v);

        //Load/Store the named property from the object (assert or return nullptr based on flag)
        Js::Var LoadPropertyHelper(LPCWSTR pname, Js::Var instance, bool mustExist = true);
        void StorePropertyHelper(LPCWSTR pname, Js::Var instance, Js::Var value);

        //Ensure a function is fully parsed/deserialized 
        Js::FunctionBody* ForceAndGetFunctionBody(Js::ParseableFunctionInfo* pfi);
    }

    namespace NSSnapValues
    {
        //////////////////
        //Helpers for TTDVars

        //serialize the TTDVar
        void EmitTTDVar(TTDVar var, FileWriter* writer, NSTokens::Separator separator);

        //de-serialize the TTDVar
        TTDVar ParseTTDVar(bool readSeperator, FileReader* reader);

        //////////////////

        //A structure for the primitive javascript types
        struct SnapPrimitiveValue
        {
            //The id (address of the object)
            TTD_PTR_ID PrimitiveValueId;

            //The type of the primitive value
            NSSnapType::SnapType* SnapType;

            //The tag given to uniquely identify this object accross time (for logging and callback uses)
            TTD_LOG_TAG ValueLogTag;

            //The optional well known token for this object (or INVALID)
            TTD_WELLKNOWN_TOKEN OptWellKnownToken;

            union
            {
                BOOL u_boolValue;
                int64 u_int64Value;
                uint64 u_uint64Value;
                double u_doubleValue;
                LPCWSTR u_stringValue;
                Js::PropertyId u_propertyIdValue; //for a symbol
            };
        };

        void ExtractSnapPrimitiveValue(SnapPrimitiveValue* snapValue, Js::RecyclableObject* jsValue, bool isWellKnown, bool isLogged, const TTDIdentifierDictionary<TTD_PTR_ID, NSSnapType::SnapType*>& idToTypeMap, SlabAllocator& alloc);
        void InflateSnapPrimitiveValue(const SnapPrimitiveValue* snapValue, InflateMap* inflator);

        void EmitSnapPrimitiveValue(const SnapPrimitiveValue* snapValue, FileWriter* writer, NSTokens::Separator separator);
        void ParseSnapPrimitiveValue(SnapPrimitiveValue* snapValue, bool readSeperator, FileReader* reader, SlabAllocator& alloc, const TTDIdentifierDictionary<TTD_PTR_ID, NSSnapType::SnapType*>& ptrIdToTypeMap);

        //////////////////

        //If a scope is a slot array this class encodes the slot information and original address of the slot array
        struct SlotArrayInfo
        {
            //The unique id for the slot array scope entry 
            TTD_PTR_ID SlotId;

            //The tag of the script context that this slot array is associated with
            TTD_LOG_TAG ScriptContextTag;

            //The number of slots in the scope array
            uint32 SlotCount;

            //The data values for the slots in the scope entry
            TTDVar* Slots;

#if ENABLE_TTD_INTERNAL_DIAGNOSTICS
            LPCWSTR* DebugSlotNameArray;
#endif

            //The function body for the function that this entry corresponds to 
            TTD_PTR_ID FunctionBodyId;
        };

        void ExtractSlotArray(SlotArrayInfo* slotInfo, Js::Var* scope, SlabAllocator& alloc);
        Js::Var* InflateSlotArrayInfo(const SlotArrayInfo* slotInfo, InflateMap* inflator);

        void EmitSlotArrayInfo(const SlotArrayInfo* slotInfo, FileWriter* writer, NSTokens::Separator separator);
        void ParseSlotArrayInfo(SlotArrayInfo* slotInfo, bool readSeperator, FileReader* reader, SlabAllocator& alloc);

        //A struct to represent a single scope entry for a function
        struct ScopeInfoEntry
        {
            //The tag indicating what type of scope entry this is
            Js::ScopeType Tag;

            //The id for looking up the entry data
            //Either ptr object for Activation and With Scopes or the slotid for SlotArray Scopes
            TTD_PTR_ID IDValue;
        };

        //A class to represent a scope list for a script function
        struct ScriptFunctionScopeInfo
        {
            //The unique id for the scope array associated with this function
            TTD_PTR_ID ScopeId;

            //The id of the script context that this slot array is associated with
            TTD_LOG_TAG ScriptContextTag;

            //The number of scope entries
            uint16 ScopeCount;

            //The array of scope entries this function has
            ScopeInfoEntry* ScopeArray;
        };

        void ExtractScriptFunctionScopeInfo(ScriptFunctionScopeInfo* funcScopeInfo, Js::FrameDisplay* environment, SlabAllocator& alloc);
        Js::FrameDisplay* InflateScriptFunctionScopeInfo(const ScriptFunctionScopeInfo* funcScopeInfo, InflateMap* inflator);

        void EmitScriptFunctionScopeInfo(const ScriptFunctionScopeInfo* funcScopeInfo, FileWriter* writer, NSTokens::Separator separator);
        void ParseScriptFunctionScopeInfo(ScriptFunctionScopeInfo* funcScopeInfo, bool readSeperator, FileReader* reader, SlabAllocator& alloc);

        //////////////////

        //A struct that we can use to resolve a function body in a shaddow context during inflation
        struct FunctionBodyResolveInfo
        {
            //The id this body is associated with
            TTD_PTR_ID FunctionBodyId;

            //The context this body is associated with
            TTD_LOG_TAG ScriptContextTag;

            //The ptr id of the parent function body (invalid if this is a top level body)
            TTD_PTR_ID ParentBodyId;

            //True if the body is from an "eval"
            bool IsEval;

            //True if the body is from a "Function" constructor
            bool IsDynamic;

            //True if this body corresponds to code not associated with a url (and not dynamic or eval)
            bool IsRuntime;

            //The string name of the function
            LPCWSTR FunctionName;

            //The uri of the source file
            LPCWSTR OptSrcUri;

            //The line number the function is def starts on
            int64 OptLine;

            //The column number the function is def starts on
            int64 OptColumn;

            //The known path to a function with the desired body
            TTD_WELLKNOWN_TOKEN OptKnownPath;

            //If this is a top level body then this is the src uri, document id, and code for it
            DWORD_PTR OptDocumentID; 
            LPCWSTR OptTopLevelSourceUri;
            LPCWSTR OptTopLevelSourceCode;
        };

        void ExtractTopLevelLoadedScriptFunctionBodyInfo(FunctionBodyResolveInfo* fbInfo, Js::FunctionBody* fb, LPCWSTR srcCode, SlabAllocator& alloc);
        void ExtractFunctionBodyInfo(FunctionBodyResolveInfo* fbInfo, Js::FunctionBody* fb, bool isWellKnown, SlabAllocator& alloc);

        void InflateFunctionBody(const FunctionBodyResolveInfo* fbInfo, InflateMap* inflator, const TTDIdentifierDictionary<TTD_PTR_ID, FunctionBodyResolveInfo*>& idToFbResolveMap);

        void EmitFunctionBodyInfo(const FunctionBodyResolveInfo* fbInfo, LPCWSTR sourceDir, IOStreamFunctions& streamFunctions, FileWriter* writer, NSTokens::Separator separator);
        void ParseFunctionBodyInfo(FunctionBodyResolveInfo* fbInfo, bool readSeperator, LPCWSTR sourceDir, IOStreamFunctions& streamFunctions, FileReader* reader, SlabAllocator& alloc);

        //Support for parsing and inflating root function bodies 
        Js::JavascriptFunction* InflateRootScriptFunction(const FunctionBodyResolveInfo* rootFbInfo, Js::ScriptContext* ctx);

        //////////////////

        //A structure for serializing/tracking script contexts
        struct SnapContext
        {
            //The tag id of the script context (actually the global object associated with this context)
            TTD_LOG_TAG m_scriptContextTagId;

            //The random seed for the context
            uint64 m_randomSeed;

            //The main URI of the context
            LPCWSTR m_contextSRC;

            //A list of all *root* scripts that have been loaded into this context
            uint32 m_loadedScriptCount;
            FunctionBodyResolveInfo* m_rootScriptArray;

            //A list of all the root objects in this context
            uint32 m_rootCount;
            TTD_PTR_ID* m_rootArray;
        };

        void ExtractScriptContext(SnapContext* snapCtx, Js::ScriptContext* ctx, SlabAllocator& alloc);
        void InflateScriptContext(const SnapContext* snpCtx, Js::ScriptContext* intoCtx, InflateMap* inflator);
        void ReLinkRoots(const SnapContext* snpCtx, Js::ScriptContext* intoCtx, InflateMap* inflator);

        void EmitSnapContext(const SnapContext* snapCtx, LPCWSTR sourceDir, IOStreamFunctions& streamFunctions, FileWriter* writer, NSTokens::Separator separator);
        void ParseSnapContext(SnapContext* intoCtx, bool readSeperator, LPCWSTR sourceDir, IOStreamFunctions& streamFunctions, FileReader* reader, SlabAllocator& alloc);
    }
}

#endif
