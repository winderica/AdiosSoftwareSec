import { Button, MenuItem, Table, TableBody, TableCell, TableHead, TableRow, TextField } from '@material-ui/core';
import { useSnackbar } from 'notistack';
import React, { ChangeEvent, useState } from 'react';
import codes from '../../samples/R4.json';
import { useStyles } from '../../styles/style';
import { getTypes } from '../../utils/getTypes';
import { Expansion } from '../Expansion';

interface Result {
    description: string;
    function: string;
    lineNumber: number;
    lineOffset: number;
}

export const R4 = () => {
    const { enqueueSnackbar } = useSnackbar();
    const [result, setResult] = useState<Result[]>([]);
    const classes = useStyles();
    const [code1, setCode1] = useState(0);
    const [types, setTypes] = useState('');

    const run = () => {
        try {
            setResult(JSON.parse(window.main.stackOverflow(codes[code1], getTypes(types))));
        } catch ({ message }) {
            enqueueSnackbar(message, { variant: 'error' });
        }
    };

    const setCode = (event: ChangeEvent<{ name?: string; value: unknown }>) => {
        setCode1(event.target.value as number);
        setResult([]);
    };

    const handleInput = (event: ChangeEvent<HTMLInputElement>) => {
        setTypes(event.target.value);
    };

    return (
        <div>
            <Expansion code1={codes[code1]} />
            <div className={classes.itemContainer}>
                <TextField
                    select
                    label='code1'
                    value={code1}
                    onChange={setCode}
                >
                    {codes.map((i, j) => <MenuItem key={j} value={j}>{j}</MenuItem>)}
                </TextField>
                <TextField
                    label='types'
                    value={types}
                    onChange={handleInput}
                />
                <Button onClick={run}>Detect Stack Overflow</Button>
            </div>
            <div className={classes.resultContainer}>
                <Table>
                    <TableHead>
                        <TableRow>
                            <TableCell>function</TableCell>
                            <TableCell>lineNumber</TableCell>
                            <TableCell>lineOffset</TableCell>
                            <TableCell>description</TableCell>
                        </TableRow>
                    </TableHead>
                    <TableBody>
                        {result.map((i, j) => (
                            <TableRow key={j}>
                                <TableCell component='th' scope='row'>{i.function}</TableCell>
                                <TableCell>{i.lineNumber}</TableCell>
                                <TableCell>{i.lineOffset}</TableCell>
                                <TableCell>{i.description}</TableCell>
                            </TableRow>
                        ))}
                    </TableBody>
                </Table>
            </div>
        </div>
    );
};
