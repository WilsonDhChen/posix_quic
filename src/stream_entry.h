#pragma once

#include <memory>
#include "fwd.h"
#include "entry.h"
#include "stream.h"
#include "fd_factory.h"
#include "fd_manager.h"
#include "socket_entry.h"

namespace posix_quic {

typedef std::shared_ptr<QuartcStream> QuartcStreamPtr;

class QuicStreamEntry
    : public EntryBase,
    public std::enable_shared_from_this<QuicStreamEntry>,
    private net::QuartcStreamInterface::Delegate
{
public:
    QuicStreamEntry(QuicSocketEntryPtr socketEntry, QuicStreamId streamId);

    EntryCategory Category() const override { return EntryCategory::Stream; }

    ssize_t Writev(const struct iovec* iov, size_t iov_count, bool fin = false);

    ssize_t Readv(const struct iovec* iov, size_t iov_count);

    int Shutdown(int how);

    int Close();

public:
    template <typename ... Args>
    static QuicStreamEntryPtr NewQuicStream(Args && ... args) {
        int fd = GetFdFactory().Alloc();
        QuicStreamEntryPtr ptr(new QuicStreamEntry(std::forward<Args>(args)...));
        ptr->SetFd(fd);
        GetFdManager().Put(fd, ptr);
        return ptr;
    }
    static void DeleteQuicStream(QuicStreamEntryPtr const& ptr);

    // -----------------------------------------------------------------
    // QuartcStreamInterface::Delegate
private:
    void OnDataAvailable() override;

    void OnClose(QuartcStreamInterface* stream) override;

    void OnBufferChanged(QuartcStreamInterface* stream) override {}

    void OnCanWriteNewData() override;
    // -----------------------------------------------------------------

private:
    QuartcStreamPtr GetQuartcStream();

private:
    QuicSocketEntryWeakPtr socketEntry_;
    QuicStreamId streamId_;
};


} // namespace posix_quic