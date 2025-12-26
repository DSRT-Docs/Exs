#ifndef EXS_LICENSE_HPP
#define EXS_LICENSE_HPP

/*
 * Copyright [2024] [DSRT]
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

namespace exs {
namespace license {

constexpr const char* notice = R"(
Exs Platform Library
Copyright [2024] [DSRT]

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
)";

constexpr const char* short_notice = 
    "Exs Platform Library - Licensed under Apache License 2.0";

void print_license_notice() {
    std::cout << notice << std::endl;
}

void print_short_notice() {
    std::cout << short_notice << std::endl;
}

} // namespace license
} // namespace exs

#endif // EXS_LICENSE_HPP
