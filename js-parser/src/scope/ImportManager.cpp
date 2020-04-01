//
// Created by Duzhong Chen on 2020/3/22.
//

#include "ImportManager.h"
#include "../parser/SyntaxNodes.h"

namespace jetpack {

    ImportManager::EC ImportManager::ResolveImportDecl(const Sp<ImportDeclaration>& importDecl) {
        for (auto& spec : importDecl->specifiers) {
            switch (spec->type) {

                /**
                 * import named specifier
                 */
                case SyntaxNodeType::ImportSpecifier: {
                    auto importSepc = std::dynamic_pointer_cast<ImportSpecifier>(spec);

                    ImportIdentifierInfo importInfo;
                    importInfo.is_namespace = false;
                    importInfo.local_name = importSepc->local->name;
                    importInfo.source_name = importSepc->imported->name;
                    importInfo.module_name = importDecl->source->raw;

                    id_map[importInfo.local_name] = importInfo;

                    break;
                }

                case SyntaxNodeType::ImportDefaultSpecifier: {
                    auto importDefault = std::dynamic_pointer_cast<ImportDefaultSpecifier>(spec);

                    ImportIdentifierInfo importInfo;
                    importInfo.is_namespace = false;
                    importInfo.local_name = importDefault->local->name;
                    importInfo.source_name = u"default";
                    importInfo.module_name = importDecl->source->raw;

                    id_map[importInfo.local_name] = importInfo;

                    break;
                }

                case SyntaxNodeType::ImportNamespaceSpecifier: {
                    auto importNP = std::dynamic_pointer_cast<ImportNamespaceSpecifier>(spec);

                    ImportIdentifierInfo importInfo;
                    importInfo.is_namespace = true;
                    importInfo.local_name = importNP->local->name;
                    importInfo.module_name = importDecl->source->raw;

                    id_map[importInfo.local_name] = importInfo;

                    break;
                }

                default:
                    return EC::UnknownSpecifier;

            }
        }

        return EC::Ok;
    }

}
