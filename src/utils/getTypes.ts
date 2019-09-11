export const getTypes = (types: string) => {
    return types.split(',').map((i) => i.trim()).filter(Boolean);
};
