package jh.craft.interpreter.errors;

public interface Result<V, E> {

    V value();

    E error();

    default boolean isOk() {
        return error() == null;
    }


    static <V, E> Result<V, E> error( E error ){
        return new ErrResult<>(error);
    }

    static <V, E> Result<V, E> ok( V value ) {
        return new OkResult<>(value);
    }

    static <V, E> Result<V, E> ok() {
        return new OkResult<>(null);
    }

}
class OkResult<V, E> implements Result<V, E> {

    private final V value;
    OkResult(V value){
        this.value = value;
    }

    @Override
    public V value() {
        return value;
    }

    @Override
    public E error() {
        return null;
    }
}

class ErrResult<V, E> implements Result<V, E> {

    private final E error;
    ErrResult(E error){
        this.error = error;
    }

    @Override
    public V value() {
        throw new RuntimeException("Error result " + error());
    }

    @Override
    public E error() {
        return error;
    }
}
