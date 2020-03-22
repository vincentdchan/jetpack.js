//
// Created by Duzhong Chen on 2020/3/22.
//

#import "../parser/SyntaxNodes.h"

namespace rocket_bundle {

    ImportManager::EC ImportManager::ResolveImportDecl(const Sp<ImportDeclaration>& import_decl) {
        for (auto& spec : import_decl->specifiers) {
            switch (spec->type) {

                /**
                 * import named specifier
                 */
                case SyntaxNodeType::ImportSpecifier: {
                    auto import_sepc = std::dynamic_pointer_cast<ImportSpecifier>(spec);

                    break;
                }

                case SyntaxNodeType::ImportDefaultSpecifier: {
                    auto import_default = std::dynamic_pointer_cast<ImportDefaultSpecifier>(spec);

                    break;
                }

                case SyntaxNodeType::ImportNamespaceSpecifier: {
                    auto import_np = std::dynamic_pointer_cast<ImportNamespaceSpecifier>(spec);

                    break;
                }

                default:
                    return EC::UnknownSpecifier;

            }
        }

        return EC::Ok;
    }

}
