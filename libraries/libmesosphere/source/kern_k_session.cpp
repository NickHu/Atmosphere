/*
 * Copyright (c) 2018-2020 Atmosphère-NX
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <mesosphere.hpp>

namespace ams::kern {

    void KSession::Initialize(KClientPort *client_port, uintptr_t name) {
        MESOSPHERE_ASSERT_THIS();

        /* Increment reference count. */
        this->Open();

        /* Create our sub sessions. */
        KAutoObject::Create(std::addressof(this->server));
        KAutoObject::Create(std::addressof(this->client));

        /* Initialize our sub sessions. */
        this->server.Initialize(this);
        this->client.Initialize(this);

        /* Set state and name. */
        this->state = State::Normal;
        this->name  = name;

        /* Set our owner process. */
        this->process = GetCurrentProcessPointer();
        this->process->Open();

        /* Set our port. */
        this->port = port;
        if (this->port != nullptr) {
            this->port->Open();
        }

        /* Mark initialized. */
        this->initialized = true;
    }

    void KSession::Finalize() {
        if (this->port != nullptr) {
            this->port->OnSessionFinalized();
            this->port->Close();
        }
    }

    void KSession::OnServerClosed() {
        MESOSPHERE_ASSERT_THIS();

        if (this->state == State::Normal) {
            this->state = State::ServerClosed;
            this->client.OnServerClosed();
        }
    }

    void KSession::OnClientClosed() {
        MESOSPHERE_ASSERT_THIS();

        if (this->state == State::Normal) {
            this->state = State::ClientClosed;
            this->server.OnClientClosed();
        }
    }

    void KSession::PostDestroy(uintptr_t arg) {
        /* Release the session count resource the owner process holds. */
        KProcess *owner = reinterpret_cast<KProcess *>(arg);
        owner->ReleaseResource(ams::svc::LimitableResource_SessionCountMax, 1);
        owner->Close();
    }

}
